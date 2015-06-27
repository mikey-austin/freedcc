/**
 * @file io.h
 * @brief Defines the computer communication API.
 * @author Mikey Austin
 * @date 2010-2011
 *
 * This module defines the communication API between the host and the
 * command station.
 */
 
#ifndef IO_DEFINED
#define IO_DEFINED

#include "dcc.h"

/** 
 * Initialise the IO module
 * 
 * This function sets up IO module buffer and USART functionality.
 */
extern void IO_module_init(void);

/** 
 * Get a new packet from host.
 *
 * This function checks the IO module buffer to see if a new packet has
 * been received.
 *
 * @return Returns a DCC packet if one was received, NULL otherwise.
 */
extern DCC_packet_T IO_read(void);

#endif
