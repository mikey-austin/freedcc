/**
 * @file dsl.h
 * @brief Defines the command station DSL parser API.
 * @author Mikey Austin
 * @date 2012-2013
 *
 * This module defines a recursive descent parser and token scanner. The
 * parser expects a human-readable domain specific language (DSL) syntax,
 * which at current, constructs the equivalent DCC packet.
 *
 * This scanner/parser module is handwritten (as opposed to generated) to
 * achieve the speed and compactness required by embedded environments.
 *
 * The grammar is as follows (in BNF form, terminals uppercase):
 *
 * @code
 * command : raw
 *         | show
 *         | cache
 *         | forward
 *         | reverse
 *         | stop
 *         ;
 *
 * raw : RAW HEX
 *     ;
 *
 * show : SHOW STATUS
 *      ;
 *
 * cache : CACHE CLEAR
 *       : CACHE SHOW NUMBER
 *       ;
 *
 * forward : FORWARD addr speed
 *         ;
 *
 * reverse : REVERSE addr speed
 *         ;
 *
 * stop : STOP addr
 *      | STOP ALL
 *      | STOP
 *      ;
 *
 * addr : ADDR NUMBER
 *      ;
 *
 * speed : SPEED NUMBER
 *       ;
 *
 * help : HELP
 *      ;
 * @endcode
 */

#ifndef DSL_INCLUDED
#define DSL_INCLUDED

#include <stdio.h>

#include "dcc.h"
#include "sys.h"

#define T                  DSL_result_T
#define DSL_RES_TYPE_UNDEF 0
#define DSL_RES_TYPE_DCC   1
#define DSL_RES_TYPE_SYS   2
#define DSL_RES_TYPE_RAW   3
#define DSL_PARSE_OK       1
#define DSL_PARSE_ERROR    0

/**
 * Structure to allow for arbitrary return types from the parser. The
 * type field determines which union member to use to access the data.
 */
typedef struct T *T;
struct T
{
    int type;
    union
    {
        DCC_packet_T packet;
        Sys_cmd_T cmd;
    } payload;
};

/**
 * Initialise the DSL module internals.
 *
 * This function in particular sets the character stream handling
 * functions used by the token scanner. These callbacks should
 * operate on the same input character stream. They allow the calling
 * context to define where the character stream is coming from, and how
 * to manage it (e.g. designed to work with embedded environments in
 * particular.
 *
 * @param flush The callback to flush the IO buffers
 */
extern void DSL_module_init(void (*flush)(void));

/**
 * Start parsing the input stream.
 *
 * This function instructs the parser module to start processing the input
 * character stream until:
 *
 * - a valid command is parsed, or
 * - a syntax error is encountered
 *
 * The argument should be a pointer to a pointer to a DSL result object. The
 * object will be created and initialised if the pointer to the pointer is
 * not NULL. If it is NULL, then the parser will essentially just perform
 * a syntax check.
 *
 * @param packet A pointer to a DSL result object pointer.
 *
 * @return 1 if the stream is parsed with no problems, 0 otherwise.
 */
extern int DSL_parser_start(T *result);

#undef T
#endif
