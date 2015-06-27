/**
 * @file sys.h
 * @brief Defines system specific accounting and monitoring functions.
 * @author Mikey Austin
 * @date 2012-2013
 */
 
#ifndef DEFINED_SYS
#define DEFINED_SYS

#include "dcc.h"

#define T                        Sys_cmd_T
#define SYS_CMD_TYPE_STATUS      0x01
#define SYS_CMD_TYPE_HELP        0x02
#define SYS_CMD_TYPE_CACHE_CLEAR 0x03
#define SYS_CMD_TYPE_CACHE_SHOW  0x04

typedef struct T *T;
struct T
{
    uint8_t type;
    void (*call)(void *args);
    void *args;
};

extern void Sys_init(void);
extern T Sys_cmd_create(uint8_t type, void *args);
extern void Sys_cmd_destroy(T cmd, void (*free_args)(void *args));
extern void Sys_process_dcc_tx(DCC_packet_T packet);
extern void Sys_process_sys_cmd(T cmd);
extern void Sys_parse_err_increment(void);
extern void Sys_parse_ok_increment(void);

#undef T
#endif
