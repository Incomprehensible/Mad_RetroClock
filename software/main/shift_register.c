#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "shift_register.h"
#include "pins.h"

#define CLK_DELAY 0

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

static void shift_bit(bool value, uint8_t bits, uint32_t delay)
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

void set_minutes(uint8_t A, uint8_t B)
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

    //printf("\nShifting:\n[ ");
    for (int i=0; i < 8; ++i)
    {
        printf("%d ", (tmp >> i) & 0x01);
        fflush(NULL);
        shift_bit((tmp >> i) & 0x01, 1, 0);
        //vTaskDelay(1000);
    }
    //printf("]\n");
    // if (nixie_offset)
    // {
    //     for (int i=0; i < 4; ++i)
    //     {
    //         shift_bit(0, 1, 0);
    //     }
    // }
    //vTaskDelay(1000);
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

    //printf("\nShifting:\n[ ");
    for (int i=0; i < 4; ++i)
    {
        printf("%d ", (tmp >> i) & 0x01);
        fflush(NULL);
        shift_bit((tmp >> i) & 0x01, 1, 0);
        //vTaskDelay(1000);
    }
    //printf("]\n");
    if (nixie_offset)
    {
        for (int i=0; i < 4; ++i)
        {
            shift_bit(0, 1, 0);
        }
    }
    //vTaskDelay(1000);
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