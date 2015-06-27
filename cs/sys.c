/**
 * @file sys.c
 * @brief Implements system specific accounting and monitoring functions.
 * @author Mikey Austin
 * @date 2012-2013
 */

#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "cache.h"
#include "sys.h"

#define T               Sys_cmd_T
#define SYS_TOT_MEM     4096

static int Sys_tx_count;
static int Sys_tx_bytes_count;
static int Sys_parse_err_count;
static int Sys_parse_ok_count;
static int Sys_sys_cmd_count;

static void Sys_cmd_status(void *args);
static void Sys_cmd_help(void *args);
static void Sys_cmd_cache_clear(void *args);
static void Sys_cmd_cache_show(void *args);

extern void
Sys_init(void)
{
    Sys_tx_count = 0;
    Sys_tx_bytes_count = 0;
    Sys_parse_err_count = 0;
    Sys_parse_ok_count = 0;
    Sys_sys_cmd_count = 0;
}

extern T
Sys_cmd_create(uint8_t type, void *args)
{
    T cmd;

    cmd = (T) malloc(sizeof(*cmd));
    if(cmd == NULL)
    {
        return NULL;
    }

    cmd->type = type;
    cmd->args = args;
    switch(cmd->type)
    {
        case SYS_CMD_TYPE_STATUS:
            cmd->call = Sys_cmd_status;
            break;

        case SYS_CMD_TYPE_HELP:
            cmd->call = Sys_cmd_help;
            break;

        case SYS_CMD_TYPE_CACHE_CLEAR:
            cmd->call = Sys_cmd_cache_clear;
            break;

        case SYS_CMD_TYPE_CACHE_SHOW:
            cmd->call = Sys_cmd_cache_show;
            break;

        default:
            cmd->call = NULL;
            break;
    }

    return cmd;
}

extern void
Sys_cmd_destroy(T cmd, void (*free_args)(void *args))
{
    if(cmd)
    {
        if(cmd->args != NULL && free_args != NULL)
        {
            free_args(cmd->args);
        }
        free(cmd);
    }
}

extern void
Sys_process_dcc_tx(DCC_packet_T packet)
{
    Sys_tx_count++;
    Sys_tx_bytes_count += packet->size;
}

extern void
Sys_process_sys_cmd(T cmd)
{
    Sys_sys_cmd_count++;
}

extern void
Sys_parse_err_increment(void)
{
    Sys_parse_err_count++;
}

extern void
Sys_parse_ok_increment(void)
{
    Sys_parse_ok_count++;
}

static void
Sys_cmd_status(void *args)
{
    int v, mem_free, cache_total, cache_used;
    double mem_free_percentage;
    extern int __heap_start, *__brkval;

    /* Calculate free heap memory. */
    mem_free = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    mem_free_percentage = (mem_free / (double) SYS_TOT_MEM) * 100;

    /* Output all counters. */
    printf_P(PSTR("system details\n"));
    printf_P(PSTR("  mem_used_bytes:\t%d\n"), (SYS_TOT_MEM - mem_free));
    printf_P(PSTR("  mem_free_bytes:\t%d\n"), mem_free);
    printf_P(PSTR("  mem_free_percent:\t%.2f%%\n"), mem_free_percentage);
    printf_P(PSTR("  sys_cmd_total:\t%d\n"), Sys_sys_cmd_count);
    printf_P(PSTR("  dcc_tx_packets:\t%d\n"), Sys_tx_count);
    printf_P(PSTR("  dcc_tx_bytes:\t\t%d\n"), Sys_tx_bytes_count);
    printf_P(PSTR("  parse_errors:\t\t%d\n"), Sys_parse_err_count);
    printf_P(PSTR("  parse_ok:\t\t%d\n"), Sys_parse_ok_count);
    printf_P(PSTR("  parse_total:\t\t%d\n"), (Sys_parse_ok_count + Sys_parse_err_count));
    printf_P(PSTR("  cache_used:\t\t%d/%d\n"), (cache_used = Cache_report_current_size()),
             (cache_total = Cache_report_total_size()));
    printf_P(PSTR("  cache_free_percent:\t%.2f%%\n\n"),
             ((cache_total - cache_used) / (double) cache_total) * 100);
}

static void
Sys_cmd_help(void *args)
{
    printf_P(PSTR("system help message goes here...\n\n"));
}

static void
Sys_cmd_cache_clear(void *args)
{
    int curr;
    curr = Cache_report_current_size();
    Cache_clear();
    printf_P(PSTR("%d item(s) purged\n\n"), curr);
}

static void
Sys_cmd_cache_show(void *args)
{
    int *address = (int*) args;
    char *hex_dump, *binary_dump;
    DCC_packet_T cached;

    if((cached = Cache_get(*address)) == NULL)
    {
        printf_P(PSTR("no cached packet for loco with address %d\n\n"), *address);
    }
    else
    {
        hex_dump = DCC_hex_dump(cached);
        binary_dump = DCC_dump(cached);
        printf_P(PSTR("cached packet details\n"));
        printf_P(PSTR("  address:\t%d\n"), *address);
        printf_P(PSTR("  speed:\t%d\n"), DCC_get_speed_step(cached));
        printf_P(PSTR("  direction:\t%s\n"), (DCC_get_direction(cached) ? "forward" : "reverse"));
        printf_P(PSTR("  hex:\t\t%s\n"), hex_dump);
        printf_P(PSTR("  binary:\t%s\n\n"), binary_dump);

        if(hex_dump)
            free(hex_dump);

        if(binary_dump)
            free(binary_dump);
    }
}
