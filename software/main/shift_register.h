#ifndef SHIFT_REGISTER_H
#define SHIFT_REGISTER_H

#define A_OFFSET 0
#define B_OFFSET 2
#define C_OFFSET 3
#define D_OFFSET 1

void test_shift();
void set_number(uint8_t, uint8_t);
void set_minutes(uint8_t, uint8_t);
void zero_out_shift(uint32_t);


#endif