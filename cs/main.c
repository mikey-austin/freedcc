/**
 * @file main.c
 * @brief The main entry point for the command station.
 * @author Mikey Austin
 * @date 2010-2011
 */

#include <stdlib.h>
#include <avr/interrupt.h>

#include "dcc.h"
#include "scheduler.h"
#include "io.h"
#include "utils.h"
#include "sys.h"

int
main(int argc, char **argv)
{
    DCC_packet_T packet;

    Sys_init();
    Scheduler_module_init();
    IO_module_init();

    /* Enable interrupts. */
    sei();

    /* Power on LED. */
    LED1_ON;
    
    for(;;)
    {
        if((packet = IO_read()) != NULL)
        {
            Scheduler_add_packet(packet);
        }
    }

    return 0;
}
