/**
 * @file scheduler.c
 * @brief Implements the DCC packet scheduling API.
 * @author Mikey Austin
 * @date 2010-2011
 */

#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "ring.h"
#include "signal.h"
#include "utils.h"
#include "cache.h"
#include "scheduler.h"

#define SCHEDULER_FLUSH_PERIOD    124   /**< 8 milliseconds @ 14.7456MHz, prescaler of 1024. */
#define SCHEDULER_TX_QUEUE_LEN    20

/**
 * Queue to hold new packets that are to be sent.
 */
static Ring_T Scheduler_tx_queue;

/**
 * An initialised idle packet, to prevent having to allocate
 * heap memory every time an idle is to be sent.
 */
static DCC_packet_T Scheduler_idle_packet;

/**
 * An uninitialised broadcast stop packet, to prevent having to allocate
 * heap memory every time a stop packet is to be sent.
 */
static DCC_packet_T Scheduler_stop_packet;

extern void
Scheduler_module_init(void)
{
    Signal_module_init();
    Cache_module_init();

    /* Set up the transmit queue for new packets. */
    Scheduler_tx_queue = Ring_create(RING_TYPE_PACKET, SCHEDULER_TX_QUEUE_LEN);

    /* Set up an idle packet. */
    Scheduler_idle_packet = DCC_baseline_packet_create();
    DCC_special_idle_packet(Scheduler_idle_packet);

    /* The parser will create this packet. */
    Scheduler_stop_packet = NULL;

    /* Set 8 bit timer to CTC mode. */
    TCCR0A |= (1 << WGM01);

    /* Set prescaler to 1024. */
    TCCR0B |= ((1 << CS02) | (1 << CS00));

    /* Enable the output compare a interupt. */
    TIMSK0 |= (1 << OCIE0A);

    /* Set the compare value for the timer. */
    OCR0A = SCHEDULER_FLUSH_PERIOD;
}

extern void
Scheduler_add_packet(DCC_packet_T packet)
{
    union Ring_data new;

    /* Prepare new ring data. */
    new.p = packet;

    /* Disable interrupts. */
    cli();

    /* Push the new packet onto the first ring. */
    Ring_push(Scheduler_tx_queue, new);

    /* Re-enable interrupts. */
    sei();

    return;
}

ISR(TIMER0_COMPA_vect)
{
    union Ring_data tx;
    DCC_packet_T cached;

    /* Initialise packets. */
    tx.p = cached = NULL;

    if(Scheduler_tx_queue->count > 0)
    {
        /*
         * There is a new packet to send.
         */
        tx = Ring_pop(Scheduler_tx_queue);
        Signal_send(tx.p->bytes, tx.p->size);

        if(Scheduler_stop_packet != NULL)
        {
            /* Destroy existing stored broadcast stop packet. */
            DCC_packet_destroy(Scheduler_stop_packet);
            Scheduler_stop_packet = NULL;
        }

        if(DCC_is_broadcast_stop(tx.p))
        {
            /* Store the broadcast stop packet for re-sending. */
            Scheduler_stop_packet = tx.p;
            Cache_clear();
        }
        else
        {
            /* Update refresh cache. */
            Cache_update(tx.p);
        }
    }
    else
    {
        /*
         * No new packets to send. Refresh previous packets
         * if they exist, or send an idle packet as a last
         * resort. A broadcast stop packet takes priority.
         */
        if(Scheduler_stop_packet != NULL)
        {
            cached = Scheduler_stop_packet;
        }
        else if((cached = Cache_get_next_packet()) == NULL)
        {
            cached = Scheduler_idle_packet;
        }

        /* Destroying of packets is handled by the cache. */
        Signal_send(cached->bytes, cached->size);
    }
}
