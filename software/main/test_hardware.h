#ifndef TEST_HARDWARE_H
#define TEST_HARDWARE_H

#include "i2cdev.h"

void test_RTC(i2c_dev_t*);
void test_DFPlayer();
void report_switch_states();

#endif