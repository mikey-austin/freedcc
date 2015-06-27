/**
 * @file signal.c
 * @brief Implements the signal modulation API.
 * @author Mikey Austin
 * @date 2010-2011
 */

#include <stdlib.h>
#include <assert.h>
#include <avr/interrupt.h> 
#include <avr/io.h>

#include "signal.h"
#include "utils.h"

#define SIGNAL_HALF_PERIOD_1        852     /**< 58 microseconds @ 14.7456MHz, prescaler of 1. */
#define SIGNAL_HALF_PERIOD_0        1617    /**< 110 microseconds @ 14.7456MHz, prescaler of 1. */

struct Signal_state
{
    unsigned char bytes[SIGNAL_MAX_BYTES];  /* Bytes to send. */
    int size;                               /* Number of bytes. */
    int cur_byte;                           /* The current byte being processed. */
    int cur_bit;                            /* The current bit being processed. */
};

static struct Signal_state Signal_state;    /* Private module state structure. */

static void Signal_generate_bit(unsigned char bit);

extern void
Signal_module_init()
{
    int i;

    /* Set up initial module state. */
    Signal_state.size       = 0;
    Signal_state.cur_byte   = 0;
    Signal_state.cur_bit    = 0;

    for(i=0; i < SIGNAL_MAX_BYTES; i++)
        Signal_state.bytes[i] = 0x00;

    /* Set the comparator as output. */
    DDRD |= SIGNAL_OUT;

    /* Set prescaler to 1 and 16 bit timer to CTC mode. */
    TCCR1B |= ((1 << CS10) | (1 << WGM12));

    /* Enable OC1A to be toggled when timer reached. */
    TCCR1A |= (1 << COM1A0);

    /* Start the signal. */
    Signal_generate_bit(1);

    /* Enable the Output Compare A interrupt. */
    TIMSK1 |= (1 << OCIE1A);
}

extern void
Signal_send(unsigned char *bytes, int size)
{
    int i;

    /* Reset the module state. */
    Signal_state.size       = size;
    Signal_state.cur_byte   = 0;
    Signal_state.cur_bit    = 7;

    assert(size <= SIGNAL_MAX_BYTES);

    /* Copy bytes into signal buffer. */
    for(i=0; i < size; i++)
        Signal_state.bytes[i] = bytes[i];

    /* Start transmission from MSB in first byte. */
    Signal_generate_bit(Signal_state.bytes[Signal_state.cur_byte]
        & (1 << Signal_state.cur_bit--));
}

static void 
Signal_generate_bit(unsigned char bit)
{
    /* If it's not zero, then it must be a one. */
    OCR1A = (bit == 0 ? SIGNAL_HALF_PERIOD_0 : SIGNAL_HALF_PERIOD_1);
}

ISR(TIMER1_COMPA_vect)
{
    /* Generate second half of bit signal with same compare value. */
    if(!(PIND & SIGNAL_OUT))
        return;

    if(Signal_state.cur_byte < Signal_state.size
        && Signal_state.cur_bit > 0)
    {
        /* Still have more of the current byte to process. */
        Signal_generate_bit(Signal_state.bytes[Signal_state.cur_byte]
            & (1 << Signal_state.cur_bit--));
    }
    else if(Signal_state.cur_byte < (Signal_state.size - 1)
        && Signal_state.cur_bit == 0)
    {
        /* On byte boundary, move on to next byte. */
        Signal_generate_bit(Signal_state.bytes[Signal_state.cur_byte++]
            & (1 << Signal_state.cur_bit));

        Signal_state.cur_bit = 7;
    }
    else if(Signal_state.cur_byte == (Signal_state.size - 1)
        && Signal_state.cur_bit == 0)
    {
        /* Transmit last bit of last byte. */
        Signal_generate_bit(Signal_state.bytes[Signal_state.cur_byte++]
            & (1 << Signal_state.cur_bit));
    }
    else
    {
        Signal_generate_bit(1);
    }
}
