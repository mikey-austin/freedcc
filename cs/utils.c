/**
 * @file utils.c
 * @brief Implements the general utilities module.
 * @author Mikey Austin
 * @date 2010-2011
 */
 
#include "init.h"
#include <avr/io.h>
#include <util/delay.h>
#include "utils.h"

extern void
blink_led(unsigned char led, int times)
{
    int i;

    UTILS_OUT_DDR = 0xFF;

    for(i=0; i < times; i++)
    {
        UTILS_OUT_PORT |= led;
        _delay_ms(25);

        UTILS_OUT_PORT &= ~led;
        _delay_ms(25);
    }
}
