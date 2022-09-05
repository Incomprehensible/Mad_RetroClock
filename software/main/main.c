/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "pins.h"
#include "shift_register.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define CLK_DELAY 0
#define TAG "I2C"

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

inline void send_clock_impulse()
{
    gpio_set_level(SHIFT_CLK_PIN, 1);
    vTaskDelay(CLK_DELAY);
    gpio_set_level(SHIFT_CLK_PIN, 0);
    vTaskDelay(CLK_DELAY);
}

void zero_out_shift(uint32_t delay)
{
    gpio_set_level(SHIFT_CLK_PIN, 0);
    gpio_set_level(SHIFT_DSA_PIN, 0);
    for (int i=0; i < 8; ++i)
    {
        send_clock_impulse();
        vTaskDelay(delay);
    }
}

void shift_bit(bool value, uint8_t bits, uint32_t delay)
{
    if (value)
        gpio_set_level(SHIFT_DSA_PIN, 1);
    else
        gpio_set_level(SHIFT_DSA_PIN, 0);
    for (int i=0; i < bits; i++)
    {
        send_clock_impulse();
        vTaskDelay(delay);
    }
}

void set_numbers(uint8_t A, uint8_t B)
{
    uint16_t tmp = 0;
    tmp |= B & 0b0001;
    tmp |= ((B >> 1) & 0b0001) << B_OFFSET;
    tmp |= ((B >> 2) & 0b0001) << C_OFFSET;
    tmp |= ((B >> 3) & 0b0001) << D_OFFSET;

    tmp |= (A & 0b0001) << 4;
    tmp |= ((A >> 1) & 0b0001) << (B_OFFSET+4);
    tmp |= ((A >> 2) & 0b0001) << (C_OFFSET+4);
    tmp |= ((A >> 3) & 0b0001) << (D_OFFSET+4);

    printf("tmp: %d\n", tmp);

    printf("\nShifting:\n[ ");
    for (int i=0; i < 8; ++i)
    {
        printf("%d ", (tmp >> i) & 0x01);
        fflush(NULL);
        shift_bit((tmp >> i) & 0x01, 1, 0);
        //vTaskDelay(1000);
    }
    printf("]\n");
    // if (nixie_offset)
    // {
    //     for (int i=0; i < 4; ++i)
    //     {
    //         shift_bit(0, 1, 0);
    //     }
    // }
    vTaskDelay(1000);
}

void set_number(uint8_t value, uint8_t nixie_offset)
{
    // val : 0b00001001
    // offset : 1
    //0b 0000 1001

    uint8_t tmp = 0;
    tmp |= value & 0b0001;
    tmp |= ((value >> 1) & 0b0001) << B_OFFSET;
    tmp |= ((value >> 2) & 0b0001) << C_OFFSET;
    tmp |= ((value >> 3) & 0b0001) << D_OFFSET;

    printf("tmp: %d\n", tmp);

    printf("\nShifting:\n[ ");
    for (int i=0; i < 4; ++i)
    {
        printf("%d ", (tmp >> i) & 0x01);
        fflush(NULL);
        shift_bit((tmp >> i) & 0x01, 1, 0);
        //vTaskDelay(1000);
    }
    printf("]\n");
    if (nixie_offset)
    {
        for (int i=0; i < 4; ++i)
        {
            shift_bit(0, 1, 0);
        }
    }
    vTaskDelay(1000);
}

void test_shift()
{
    gpio_set_level(SHIFT_CLK_PIN, 0);
    gpio_set_level(SHIFT_DSA_PIN, 1);
    gpio_set_level(SHIFT_CLK_PIN, 1);
    vTaskDelay(CLK_DELAY);
    gpio_set_level(SHIFT_CLK_PIN, 0);
    vTaskDelay(CLK_DELAY);
    gpio_set_level(SHIFT_CLK_PIN, 1);
    vTaskDelay(CLK_DELAY);
    gpio_set_level(SHIFT_CLK_PIN, 0);
    vTaskDelay(CLK_DELAY);
    vTaskDelay(1000);
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

esp_err_t i2c_master_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = 100000,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_flags = 0,
    };

    i2c_param_config(I2C_NUM_0, &conf);
    return i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

void test_i2c()
{
    // i2c init & scan
    if (i2c_master_init() != ESP_OK) {
        ESP_LOGE(TAG, "i2c init failed\n");
        return;
    }

     printf("i2c scan: \n");
     for (uint8_t i = 1; i < 127; i++)
     {
        int ret;
        i2c_cmd_handle_t cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (i << 1) | I2C_MASTER_WRITE, 1);
        i2c_master_stop(cmd);
        ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 100 / portTICK_RATE_MS);
        i2c_cmd_link_delete(cmd);
    
        if (ret == ESP_OK)
        {
            printf("Found device at: 0x%2x\n", i);
        }
    }
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

    //zero_out_shift(0);
    //set_number(1, 0);
    //set_numbers(1, 2);
    //test_shift();
    test_i2c();
    for (int i = 10; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    report_switch_states();
    
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
