#include "debug.h"
#include "pins.h"
#include "esp_log.h"

extern volatile bool ALARM_MODE_SET;
extern volatile bool NEXT_NIXIE;
extern volatile bool ALARM_SET;
extern volatile bool ALARM_UNSET;
extern volatile bool TURN_RIGHT;
extern volatile bool TURN_LEFT;
extern volatile bool BLOCK_LEFT;
extern volatile bool BLOCK_RIGHT;

void report_flags()
{
    ESP_LOGI("FLAGS", "ALARM_MODE_SET: %d", ALARM_MODE_SET);
    ESP_LOGI("FLAGS", "NEXT_NIXIE: %d", NEXT_NIXIE);
    ESP_LOGI("FLAGS", "ALARM_SET: %d", ALARM_SET);
    ESP_LOGI("FLAGS", "ALARM_UNSET: %d", ALARM_UNSET);
}