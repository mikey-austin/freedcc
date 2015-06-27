/**
 * @file utils.h
 * @brief Defines general utilities for the system.
 * @author Mikey Austin
 * @date 2010-2011
 */
 
#ifndef UTILS_DEFINED
#define UTILS_DEFINED

#include <avr/io.h>

#define LED1 (1 << PB6)
#define LED2 (1 << PB7)

#define UTILS_OUT_DDR   DDRB
#define UTILS_OUT_PORT  PORTB

#define LED1_ON     UTILS_OUT_DDR = 0xFF; (UTILS_OUT_PORT |= LED1)
#define LED1_OFF    UTILS_OUT_DDR = 0xFF; (UTILS_OUT_PORT &= ~LED1)
#define LED2_ON     UTILS_OUT_DDR = 0xFF; (UTILS_OUT_PORT |= LED2)
#define LED2_OFF    UTILS_OUT_DDR = 0xFF; (UTILS_OUT_PORT &= ~LED2)

/**
 * Blink the desired LED.
 *
 * @param led The LED to blink.
 * @param times The number of times to blink the desired LED.
 */
extern void blink_led(unsigned char led, int times);

#endif
