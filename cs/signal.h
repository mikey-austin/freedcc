/**
 * @file signal.h
 * @brief Defines the signal modulation API.
 * @author Mikey Austin
 * @date 2010-2011
 *
 * Defines the API to modulate arbitrary bytes via the avr microcontroller
 * timer interrupts. This module deals only with bytes in order to keep the
 * module self contained.
 */

#ifndef SIGNAL_DEFINED
#define SIGNAL_DEFINED

#define SIGNAL_OUT          (1 << PD5)
#define SIGNAL_MAX_BYTES    15

/** Initialise the signal module. */
extern void Signal_module_init(void);

/**
 * Modulate an arbitrary array of bytes.
 *
 * This function takes in an array of bytes and modulates each bit in sequence
 * using the avr CTC timer interrupts.
 *
 * @param bytes The array of bytes to modulate.
 * @param size The size of the bytes array.
 */
extern void Signal_send(unsigned char *bytes, int size);

#endif
