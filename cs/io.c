/**
 * @file io.c
 * @brief Implements the computer communication API.
 * @author Mikey Austin
 * @date 2010-2011
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "io.h"
#include "dsl.h"
#include "ring.h"
#include "utils.h"
#include "init.h"

#define IO_RINGSIZE              50
#define IO_BAUD_RATE             9600
#define IO_BAUD_PRESCALE         ((F_CPU + IO_BAUD_RATE * 8L) / (IO_BAUD_RATE * 16UL) - 1)
#define IO_PROMPT                "freedcc> "

static volatile Ring_T IO_rx_ring;
static FILE IO_stream;

static int IO_putc(char c, FILE *stream);
static int IO_getc(FILE *stream);
static void IO_flush(void);
static void IO_free_address(void *args);

extern void
IO_module_init(void)
{
    /* Set up USART. */
    UCSR0B |= ((1 << RXEN0) | (1 << TXEN0));

    /* Set up 8 bit transfer. */
    UCSR0C |= ((1 << UCSZ01) | (1 << UCSZ00));

    /* Set the baud prescale value. */
    UBRR0H = (IO_BAUD_PRESCALE >> 8);
    UBRR0L = IO_BAUD_PRESCALE;

    /* Set up module buffer. */
    IO_rx_ring = Ring_create(RING_TYPE_INT, IO_RINGSIZE);

    /* Setup IO stream. */
    fdev_setup_stream(&IO_stream, IO_putc, IO_getc, _FDEV_SETUP_RW);

    /* Initialise the DSL scanner & parser. */
    DSL_module_init(IO_flush);

    /* Set stdio default streams for convenience. */
    stdout = &IO_stream;
    stdin = &IO_stream;
}

extern DCC_packet_T
IO_read(void)
{
    int c;
    DSL_result_T result = NULL;
    DCC_packet_T packet = NULL;

    if(UCSR0A & (1 << RXC0))
    {
        if((c = getchar()) != '\n' && c != ' ')
        {
            ungetc(c, stdin);

            if(DSL_parser_start(&result))
            {
                /* A valid packet has been received. */
                blink_led(LED2, 1);
                Sys_parse_ok_increment();

                switch(result->type)
                {
                    case DSL_RES_TYPE_RAW:
                    case DSL_RES_TYPE_DCC:
                        printf_P(PSTR("ok\n\n"));
                        packet = result->payload.packet;
                        Sys_process_dcc_tx(packet);
                        break;

                    case DSL_RES_TYPE_SYS:
                        Sys_process_sys_cmd(result->payload.cmd);
                        result->payload.cmd->call(result->payload.cmd->args);
                        Sys_cmd_destroy(result->payload.cmd, IO_free_address);
                        break;
                }

                free(result);
            }
            else
            {
                Sys_parse_err_increment();
                printf_P(PSTR("parse error\n\n"));
            }
        }

        /* Print prompt. */
        printf_P(PSTR("\r%s"), IO_PROMPT);
    }

    return packet;
}

static int
IO_getc(FILE *stream)
{
    union Ring_data popped, rx;

    if(IO_rx_ring->count == 0)
    {
        /* Poll UART until we get a line of input. */
        for(;;)
        {
            loop_until_bit_is_set(UCSR0A, RXC0);

            if(UCSR0A & (1 << FE0))
            {
                return _FDEV_EOF;
            }
            else if(UCSR0A & (1 << DOR0))
            {
                return _FDEV_ERR;
            }

            switch((rx.i = UDR0))
            {
                case '\t':
                    rx.i = ' ';
                    break;

                case '\r':
                    rx.i = '\n';
                    break;
            }

            Ring_push(IO_rx_ring, rx);
            IO_putc(rx.i, stream);

            if(rx.i == '\n')
            {
                break;
            }
        }
    }

    popped = Ring_pop(IO_rx_ring);

    return popped.i;
}

static int
IO_putc(char c, FILE *stream)
{
    if(c == '\n')
    {
        IO_putc('\r', stream);
    }

    /* Wait until we can write. */
    loop_until_bit_is_set(UCSR0A, UDRE0);

    /* Write the byte. */
    UDR0 = c;

    return 0;
}

static void
IO_flush(void)
{
    unsigned char dummy __attribute__ ((unused));

    /* Reset rx ring. */
    Ring_reset(IO_rx_ring);

    /* Flush USART receive buffer. */
    while(UCSR0A & (1 << RXC0))
    {
        dummy = UDR0;
    }

    /* Clear stream EOF and error flags. */
    clearerror(&IO_stream);
}

static void
IO_free_address(void *args)
{
    if(args)
        free(args);
}
