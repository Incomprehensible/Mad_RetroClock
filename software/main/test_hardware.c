#include "test_hardware.h"
#include "dfplayer.h"
#include "pins.h"
#include "../RTC/ds3231.h"
#include "esp_log.h"

static const char *I2C = "I2C";

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

void test_DFPlayer()
{
    ESP_LOGI("UART", "Requested parameters...");
    vTaskDelay((3*1000) / portTICK_PERIOD_MS);
    execute_command(DFP_REQ_PARAMS, 0x0);
    await_feedback();

    ESP_LOGI("UART", "Set volume...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    execute_command(DFP_VOL, 25);
    await_feedback();
    ESP_LOGI("UART", "Requested status...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    execute_command(DFP_REQ_STATUS, 0);
    await_feedback();
    ESP_LOGI("UART", "Requested version...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    execute_command(DFP_REQ_VERS, 0);
    await_feedback();
    ESP_LOGI("UART", "Requested volume...");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    execute_command(DFP_REQ_VOL, 0);
    await_feedback();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    execute_command(DFP_TRACK, 3);
    await_feedback();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}