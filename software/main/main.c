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
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_spi_flash.h"
#include "pins.h"
#include "shift_register.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "../RTC/ds3231.h"
#include "shift_register.h"

#define CLK_DELAY 0
static const char *I2C = "I2C";

void obtain_time(struct tm*);

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

void init_RTC(i2c_dev_t *dev, struct tm *time)
{
    if (i2c_master_init(dev, I2C_NUM_0) != ESP_OK) {
        ESP_LOGE(I2C, "I2C init failed.\n");
        return;
    }
    if (ds3231_set_time(dev, time) != ESP_OK) {
        ESP_LOGE(I2C, "Couldn't set time in mini RTS module.\n");
        return;
    }
    
    if (ds3231_get_time(dev, time) != ESP_OK) {
        ESP_LOGE(I2C, "Couldn't ask the current time.\n");
        return;
    }
    ESP_LOGI(I2C, "Current time: %d : %d : %d", time->tm_hour, time->tm_min , time->tm_sec);
}

void set_minutes(uint8_t A, uint8_t B)
{
    set_numbers(A, B);
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

    power_nixie(false);

// TEST
    //zero_out_shift(0);
    //set_number(1, 0);
    //set_numbers(1, 2);
    //test_shift();
    //test_i2c();

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    //wifi_init();
    struct tm timeinfo = { 0 };
    obtain_time(&timeinfo);

    i2c_dev_t dev = {
        .port = I2C_NUM_0,
        .addr = DS3231_ADDR,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_speed = I2C_FREQ_HZ,
    };
    init_RTC(&dev, &timeinfo);

    report_switch_states();

    struct tm old_timeinfo = { 0 };
    power_nixie(true);
    while (true)
    {
        if (ds3231_get_time(&dev, &timeinfo) != ESP_OK) {
            ESP_LOGE(I2C, "Couldn't ask the current time.\n");
            break;
        }

    // if old != new
        set_hour((int)(timeinfo.tm_hour / 10), (int)(timeinfo.tm_hour % 10));
        set_minutes((int)(timeinfo.tm_min / 10), (int)(timeinfo.tm_min % 10));
        vTaskDelay(10 / portTICK_PERIOD_MS);
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
