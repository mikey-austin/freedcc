/**
 * @file scheduler.h
 * @brief Defines the DCC packet scheduling API.
 * @author Mikey Austin
 * @date 2010-2011
 *
 * This module is responsible for the following main tasks:
 *  - adding DCC packets to the appropriate priority queue
 *  - removing packets which are no longer needed
 *  - sending the packets to the <i>signal</i> modulation module
 */
 
#ifndef DEFINED_SCHEDULER
#define DEFINED_SCHEDULER

#include "dcc.h"

/**
 * Set up the Scheduler module.
 *
 * This function initialises the Scheduler module, including setting up the
 * timers for the flushing of queues to the track.
 */
extern void Scheduler_module_init(void);

/**
 * Add a new DCC packet to the appropriate queue.
 *
 * This function firstly checks for packets with the same address in the
 * queues to determine if there is room, then adds the packet to the
 * appropriate queue based on the packet priority.
 */
extern void Scheduler_add_packet(DCC_packet_T packet);

#endif
