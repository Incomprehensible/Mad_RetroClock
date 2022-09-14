#ifndef TIME_KEEPER_H
#define TIME_KEEPER_H

#include <time.h>
#include <sys/time.h>
#include "esp_sntp.h"

#define UTC_OFFSET (3 * 60 * 60)
#define TIME_SERVER "0.ru.pool.ntp.org"
#define TIME_SERVER1 "pool.ntp.org"
#define TIME_SERVER2 "1.ru.pool.ntp.org"

#define NTP_RETRY_COUNT 7

#endif