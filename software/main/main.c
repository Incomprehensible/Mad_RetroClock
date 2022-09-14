/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_spi_flash.h"
#include "pins.h"
#include "shift_register.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "../RTC/ds3231.h"
#include "shift_register.h"
#include "esp_sleep.h"

struct Alarm {
    struct tm time;
    bool set;
};

struct encoder_data {
    int clk;
    int data;
} __attribute__((PACKED));

static const char *I2C = "I2C";

volatile bool ALARM_MODE_SET = false;
volatile bool NEXT_NIXIE = false;
volatile bool ALARM_SET = false;
volatile bool ALARM_UNSET = false;
volatile bool TURN_RIGHT = false;
volatile bool TURN_LEFT = false;
// volatile int32_t turnOffset = 0;
volatile bool BLOCK_LEFT = false;
volatile bool BLOCK_RIGHT = false;

bool obtain_time(struct tm*);

QueueHandle_t handle = NULL;

static void IRAM_ATTR switch_isr_handler(void* arg)
{
    if (ALARM_MODE_SET)
        NEXT_NIXIE = true;
    else
        ALARM_MODE_SET = true;
}

static void IRAM_ATTR encoder_isr_handler(void* arg)
{
    struct encoder_data data = {
        .clk = gpio_get_level(ENC_CLK_PIN),
        .data = gpio_get_level(ENC_DATA_PIN),
    };
    BaseType_t ContextSwitchRequest = pdFALSE;
 
    xQueueSendToBackFromISR(handle,(void*)&data,&ContextSwitchRequest);
   
    if(ContextSwitchRequest){
        taskYIELD();
    }
}

// static void IRAM_ATTR encoder_isr_handler(void* arg)
// {
//     if ((gpio_get_level(ENC_CLK_PIN) == 0))
//     {
//         if ((!BLOCK_RIGHT) && (gpio_get_level(ENC_DATA_PIN) == 0))
//         {
//             BLOCK_RIGHT = true;
//             BLOCK_LEFT = false;
//         }
//         if (gpio_get_level(ENC_DATA_PIN) == 1)
//         {
//             BLOCK_LEFT = true;
//             BLOCK_RIGHT = false;
//         }
//     }
//     if (gpio_get_level(ENC_CLK_PIN) == 1)
//     {
//         if (BLOCK_RIGHT && (gpio_get_level(ENC_DATA_PIN) == 1))
//         {
//             TURN_RIGHT = true;
//             TURN_LEFT = false;
//             BLOCK_RIGHT = false;
//         }
//         else if (BLOCK_LEFT && (gpio_get_level(ENC_DATA_PIN) == 0))
//         {
//             TURN_LEFT = true;
//             TURN_RIGHT = false;
//             BLOCK_LEFT = false;
//         }
//     }
// }

static void IRAM_ATTR button_isr_handler(void* arg)
{
    if ((uint32_t)arg == BUTTON_Y_PIN) {
        ALARM_SET = true;
        ALARM_UNSET = false;
    }
    else {
        ALARM_SET = false;
        ALARM_UNSET = true;
    }
}

void init_gpio()
{
    gpio_set_direction(SHDN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(N1A_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(N1B_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(N1C_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(N1D_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(N2A_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(N2B_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(N2C_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(N2D_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SHIFT_CLK_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(SHIFT_DSA_PIN, GPIO_MODE_OUTPUT);

    gpio_set_direction(SWITCH_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(ENC_DATA_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(ENC_CLK_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_Y_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(BUTTON_N_PIN, GPIO_MODE_INPUT);

    gpio_set_level(SHDN_PIN, 1);

    gpio_set_level(N1A_PIN, 0);
    gpio_set_level(N1B_PIN, 0);
    gpio_set_level(N1C_PIN, 0);
    gpio_set_level(N1D_PIN, 0);
    gpio_set_level(N2A_PIN, 0);
    gpio_set_level(N2B_PIN, 0);
    gpio_set_level(N2C_PIN, 0);
    gpio_set_level(N2D_PIN, 0);
    
    gpio_set_level(SHIFT_DSA_PIN, 0);
    gpio_set_level(SHIFT_CLK_PIN, 0);

    if (gpio_install_isr_service(0) != ESP_OK)
    // if (gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_EDGE | ESP_INTR_FLAG_INTRDISABLED) != ESP_OK)
        ESP_LOGE("GPIO", "Error in gpio_install_isr_service()");
    gpio_set_intr_type(SWITCH_PIN, GPIO_INTR_NEGEDGE);
    if (gpio_isr_handler_add(SWITCH_PIN, switch_isr_handler, NULL) != ESP_OK)
        ESP_LOGE("GPIO", "Error in gpio_isr_handler_add() for switch");
    if (gpio_intr_enable(SWITCH_PIN) != ESP_OK)
        ESP_LOGE("GPIO", "Error in gpio_intr_enable() for switch");

    // gpio_set_intr_type(ENC_CLK_PIN, GPIO_INTR_NEGEDGE);
    // if (gpio_isr_handler_add(ENC_CLK_PIN, encoder_isr_handler, (void*) ENC_CLK_PIN) != ESP_OK)
    //     ESP_LOGE("GPIO", "Error in gpio_isr_handler_add() for enc_clk");
    // gpio_set_intr_type(ENC_DATA_PIN, GPIO_INTR_NEGEDGE);
    // gpio_isr_handler_add(ENC_DATA_PIN, encoder_isr_handler, (void*) ENC_DATA_PIN);
    gpio_set_intr_type(ENC_CLK_PIN, GPIO_INTR_ANYEDGE);
    if (gpio_isr_handler_add(ENC_CLK_PIN, encoder_isr_handler, (void*) ENC_CLK_PIN) != ESP_OK)
        ESP_LOGE("GPIO", "Error in gpio_isr_handler_add() for enc_clk");
    
    gpio_set_intr_type(BUTTON_Y_PIN, GPIO_INTR_NEGEDGE);
    if (gpio_isr_handler_add(BUTTON_Y_PIN, button_isr_handler, (void*) BUTTON_Y_PIN) != ESP_OK)
        ESP_LOGE("GPIO", "Error in gpio_isr_handler_add() for button_y");
    gpio_set_intr_type(BUTTON_N_PIN, GPIO_INTR_NEGEDGE);
    gpio_isr_handler_add(BUTTON_N_PIN, button_isr_handler, (void*) BUTTON_N_PIN);
    gpio_intr_disable(ENC_DATA_PIN);
    gpio_intr_disable(ENC_CLK_PIN);
    gpio_intr_disable(BUTTON_Y_PIN);
    gpio_intr_disable(BUTTON_N_PIN);
}

void power_nixie(bool on)
{
    if (on)
        gpio_set_level(SHDN_PIN, 0);
    else
        gpio_set_level(SHDN_PIN, 1);
}

void report_switch_states()
{
    if (gpio_get_level(SWITCH_PIN) == 0)
        printf("\nSWITCH LOW.\n");
    else
        printf("\nSWITCH HIGH.\n");

    if (gpio_get_level(BUTTON_Y_PIN) == 0)
        printf("\nBUTTON Y LOW.\n");
    else
        printf("\nBUTTON Y HIGH.\n");
    
    if (gpio_get_level(BUTTON_N_PIN) == 0)
        printf("\nBUTTON N LOW.\n");
    else
        printf("\nBUTTON N HIGH.\n");

    if (gpio_get_level(ENC_CLK_PIN) == 0)
        printf("\nENC CLK LOW.\n");
    else
        printf("\nENC CLK HIGH.\n");

    if (gpio_get_level(ENC_DATA_PIN) == 0)
        printf("\nENC DATA LOW.\n");
    else
        printf("\nENC DATA HIGH.\n");
}

esp_err_t uart_init()
{
    const uart_port_t uart_num = UART_NUM;
    esp_err_t err;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
    };

    // Install UART driver using an event queue here
    ESP_ERROR_CHECK(uart_driver_install(uart_num, UART_BUFF_SIZE, \
                                        UART_BUFF_SIZE, UART_QUEUE_SIZE, &uart_queue, 0));
    err = uart_param_config(uart_num, &uart_config);
    if (err != ESP_OK)
        return err;
    
    err = uart_set_pin(uart_num, UART_TX, UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (err != ESP_OK)
        return err;
 
    err = uart_set_mode(uart_num, UART_MODE_UART);
    if (err != ESP_OK)
        return err;
    
    esp_log_level_set("UART", ESP_LOG_INFO);
    return ESP_OK;
}

// esp_err_t i2c_master_init()
// {
//     i2c_config_t conf = {
//         .mode = I2C_MODE_MASTER,
//         .sda_pullup_en = GPIO_PULLUP_DISABLE,
//         .scl_pullup_en = GPIO_PULLUP_DISABLE,
//         .master.clk_speed = 400000,
//         .sda_io_num = I2C_SDA_PIN,
//         .scl_io_num = I2C_SCL_PIN,
//         .clk_flags = 0,
//     };

//     i2c_param_config(I2C_NUM_0, &conf);
//     return i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
// }

esp_err_t i2c_master_init(i2c_dev_t *dev, i2c_port_t port)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = dev->clk_speed,
        .sda_io_num = dev->sda_io_num,
        .scl_io_num = dev->scl_io_num,
        .clk_flags = 0,
    };

    i2c_param_config(port, &conf);
    return i2c_driver_install(port, conf.mode, 0, 0, 0);
}

// void test_i2c()
// {
//     // i2c init & scan
//     if (i2c_master_init() != ESP_OK) {
//         ESP_LOGE(TAG, "i2c init failed\n");
//         return;
//     }

//      printf("i2c scan: \n");
//      for (uint8_t i = 1; i < 127; i++)
//      {
//         int ret;
//         i2c_cmd_handle_t cmd = i2c_cmd_link_create();
//         i2c_master_start(cmd);
//         i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
//         i2c_master_stop(cmd);
//         ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);
//         i2c_cmd_link_delete(cmd);
    
//         if (ret == ESP_OK)
//         {
//             printf("Found device at: 0x%2x\n", i);
//         }
//     }
// }

void test_RTC(i2c_dev_t *dev)
{
    struct tm time = { 0 };
    if (ds3231_get_time(dev, &time) != ESP_OK) {
        ESP_LOGE(I2C, "Couldn't ask the current time.\n");
        return;
    }
    ESP_LOGI(I2C, "Current time: %d : %d : %d", time.tm_hour, time.tm_min , time.tm_sec);
    if (ds3231_get_time(dev, &time) != ESP_OK) {
        ESP_LOGE(I2C, "Couldn't ask the current time.\n");
        return;
    }
    ESP_LOGI(I2C, "Current time: %d : %d : %d", time.tm_hour, time.tm_min , time.tm_sec);
    if (ds3231_get_time(dev, &time) != ESP_OK) {
        ESP_LOGE(I2C, "Couldn't ask the current time.\n");
        return;
    }
    ESP_LOGI(I2C, "Current time: %d : %d : %d", time.tm_hour, time.tm_min , time.tm_sec);
}

void init_RTC(i2c_dev_t *dev, struct tm *time, bool set_time)
{
    if (i2c_master_init(dev, I2C_NUM_0) != ESP_OK) {
        ESP_LOGE(I2C, "I2C init failed.\n");
        return;
    }

    if (set_time && ds3231_set_time(dev, time) != ESP_OK) {
        ESP_LOGE(I2C, "Couldn't set time in mini RTS module.\n");
        return;
    }
    
    if (ds3231_get_time(dev, time) != ESP_OK) {
        ESP_LOGE(I2C, "Couldn't ask the current time.\n");
        return;
    }
    ESP_LOGI(I2C, "Current time: %d : %d : %d", time->tm_hour, time->tm_min , time->tm_sec);
}

void set_hour(uint8_t A, uint8_t B)
{
    gpio_set_level(N1A_PIN, A & 0x0001);
    gpio_set_level(N1B_PIN, (A >> 1) & 0x0001);
    gpio_set_level(N1C_PIN, (A >> 2) & 0x0001);
    gpio_set_level(N1D_PIN, (A >> 3) & 0x0001);
    gpio_set_level(N2A_PIN, B & 0x0001);
    gpio_set_level(N2B_PIN, (B >> 1) & 0x0001);
    gpio_set_level(N2C_PIN, (B >> 2) & 0x0001);
    gpio_set_level(N2D_PIN, (B >> 3) & 0x0001);
}

void set_time(int hour, int min)
{
    set_hour(hour / 10, hour % 10);
    set_minutes(min / 10, min % 10);
}

void set_number_by_nixie(uint8_t nixie, uint8_t* number)
{
    ESP_LOGI("ALARM", "set_number_by_nixie(): nixie: %d number: %d", nixie, *number);
    switch (nixie)
    {
        case (0):
        {
            gpio_set_level(N1A_PIN, *number & 0x0001);
            gpio_set_level(N1B_PIN, (*number >> 1) & 0x0001);
            gpio_set_level(N1C_PIN, (*number >> 2) & 0x0001);
            gpio_set_level(N1D_PIN, (*number >> 3) & 0x0001);
        } break;
        case (1):
        {
            gpio_set_level(N2A_PIN, *number & 0x0001);
            gpio_set_level(N2B_PIN, (*number >> 1) & 0x0001);
            gpio_set_level(N2C_PIN, (*number >> 2) & 0x0001);
            gpio_set_level(N2D_PIN, (*number >> 3) & 0x0001);
        } break;
        case (2): {
            set_minutes(*number, *(number+1));
        } break;
        case (3): {
            set_minutes(*(number-1), *number);
        } break;
        default:
            break;
    }
}

void get_rotation(struct encoder_data *data, uint8_t* num)
{
    if (data->clk == 0)
    {
        if (!BLOCK_LEFT && (data->data == 0))
        {
            BLOCK_LEFT = true;
            BLOCK_RIGHT = false;
        }
        else if (data->data == 1)
        {
            BLOCK_RIGHT = true;
            BLOCK_LEFT = false;
        }
    }
    else
    {
        if (BLOCK_LEFT && (data->data == 1))
        {
            *num = (*num == 0)? 9 : *num-1;
            BLOCK_LEFT = false;
        }
        else if (BLOCK_RIGHT && (data->data == 0))
        {
            *num = (*num + 1 > 9)? 0 : *num+1;
            BLOCK_RIGHT = false;
        }
    }
}

void run_alarm_mode(struct Alarm* timeinfo, QueueHandle_t* handle)
{
    uint8_t index = 0;
    uint8_t number[4] = { 0 };
    // enable all interrupts
    if (gpio_intr_enable(ENC_DATA_PIN) != ESP_OK)
        ESP_LOGE("GPIO", "Error in gpio_intr_enable() for enc_data");
    gpio_intr_enable(ENC_CLK_PIN);
    gpio_intr_enable(BUTTON_Y_PIN);
    gpio_intr_enable(BUTTON_N_PIN);

    set_time(timeinfo->time.tm_hour, timeinfo->time.tm_min);
    ESP_LOGI("ALARM", "hour: %d, min: %d", timeinfo->time.tm_hour, timeinfo->time.tm_min);
    number[0] = timeinfo->time.tm_hour / 10;
    number[1] = timeinfo->time.tm_hour % 10;
    number[2] = timeinfo->time.tm_min / 10;
    number[3] = timeinfo->time.tm_min % 10;

    struct encoder_data data = {0};
    while (!ALARM_SET && !ALARM_UNSET)
    {
        while(uxQueueMessagesWaiting(*handle))
        {
            if (NEXT_NIXIE) {
                NEXT_NIXIE = false;
                index = (index + 1 > 3)? 0 : index + 1;
                //ESP_LOGI("ALARM", "run_alarm_mode(): NEXT_NIXIE");
            }
            xQueueReceive(*handle,(void*)&data,pdMS_TO_TICKS(100));
            
            get_rotation(&data, &number[index]);
            // if (TURN_RIGHT) {
            //     TURN_RIGHT = 0;
            //     number[index] = (number[index] + 1 > 9)? 0 : number[index]+1;
            // }
            // else if (TURN_LEFT) {
            //     TURN_LEFT = 0;
            //     number[index] = (number[index] == 0)? 9 : number[index]-1;
            // }
        
            ESP_LOGI("ALARM", "run_alarm_mode(): index: %d number[]: %d", index, number[index]);
            // if (NEXT_NIXIE) {
            //     NEXT_NIXIE = false;
            //     index = (index + 1 > 3)? 0 : index + 1;
            //     //ESP_LOGI("ALARM", "run_alarm_mode(): NEXT_NIXIE");
            // }
            set_number_by_nixie(index, &number[index]);
            // vTaskDelay(pdMS_TO_TICKS(10));
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
    // unregister interrupts
    gpio_intr_disable(ENC_DATA_PIN);
    gpio_intr_disable(ENC_CLK_PIN);
    gpio_intr_disable(BUTTON_Y_PIN);
    ESP_LOGI("ALARM", "run_alarm_mode(): ALARM_SET or ALARM_UNSET");
    if (ALARM_SET)
    {
        ESP_LOGI("ALARM", "run_alarm_mode(): ALARM_SET");
        timeinfo->time.tm_hour = number[0]*10+number[1];
        timeinfo->time.tm_min = number[2]*10+number[3];
        timeinfo->set = true;
        ESP_LOGI("ALARM", "SET ALARM: 0: %d, 1: %d, 2: %d, 3: %d", number[0], number[1], number[2], number[3]);
        ESP_LOGI("ALARM", "SET ALARM: hour: %d, min: %d", number[0]*10+number[1], number[2]*10+number[3]);
    }
    if (!timeinfo->set)
     // we don't need to cancel any set alarms
        gpio_intr_disable(BUTTON_N_PIN);

    ALARM_MODE_SET = false;
    NEXT_NIXIE = false;
    ALARM_SET = false;
    ALARM_UNSET = false;
}

void report_flags()
{
    ESP_LOGI("FLAGS", "ALARM_MODE_SET: %d", ALARM_MODE_SET);
    ESP_LOGI("FLAGS", "NEXT_NIXIE: %d", NEXT_NIXIE);
    ESP_LOGI("FLAGS", "ALARM_SET: %d", ALARM_SET);
    ESP_LOGI("FLAGS", "ALARM_UNSET: %d", ALARM_UNSET);
}

void app_main(void)
{
    printf("Hello world!\n");
    init_gpio();

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), WiFi%s%s, ",
            CONFIG_IDF_TARGET,
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %d bytes\n", esp_get_minimum_free_heap_size());

// TEST
    //zero_out_shift(0);
    //set_number(1, 0);
    //set_minutes(1, 2);
    //test_shift();
    //test_i2c();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    struct tm timeinfo = { 0 };
    bool synchronized = obtain_time(&timeinfo);

    i2c_dev_t dev = {
        .port = I2C_NUM_0,
        .addr = DS3231_ADDR,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_speed = I2C_FREQ_HZ,
    };
    init_RTC(&dev, &timeinfo, synchronized);

    struct tm old_timeinfo = { 0 };
    struct Alarm alarm_timeinfo = { 0 };

    power_nixie(true);
    report_switch_states();
    report_flags();
            handle = xQueueCreate(10,sizeof(struct encoder_data));

    while (true)
    {
        if (ALARM_UNSET)
        {
            gpio_intr_disable(BUTTON_N_PIN);
            memset(&alarm_timeinfo, 0, sizeof(struct Alarm));
            ALARM_UNSET = false;
            ESP_LOGI("ALARM", "set ALARM_UNSET");
        }
        if (ALARM_MODE_SET)
        {
            ESP_LOGI("ALARM", "entering alarm mode");
            run_alarm_mode(&alarm_timeinfo, &handle);
            ds3231_get_time(&dev, &timeinfo);
            set_time(timeinfo.tm_hour, timeinfo.tm_min);
            vTaskDelay(100);
        }
        else
        {
            if (ds3231_get_time(&dev, &timeinfo) != ESP_OK) {
                ESP_LOGE(I2C, "Couldn't ask the current time.\n");
                break;
            }

            if ((old_timeinfo.tm_min != timeinfo.tm_min) || (old_timeinfo.tm_hour != timeinfo.tm_hour))
            {
                set_hour((int)(timeinfo.tm_hour / 10), (int)(timeinfo.tm_hour % 10));
                set_minutes((int)(timeinfo.tm_min / 10), (int)(timeinfo.tm_min % 10));
                memcpy(&old_timeinfo, &timeinfo, sizeof(struct tm));
                vTaskDelay(450 / portTICK_PERIOD_MS);
            }
            else
                vTaskDelay(300 / portTICK_PERIOD_MS);
        }
        
    }
    power_nixie(false);

    
    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
