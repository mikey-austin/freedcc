/**
 * @file ring.h
 * @brief Defines a ring buffer data structure interface.
 * @author Mikey Austin
 * @date 2010-2011
 *
 * This module defines a generic ring buffer.
 */
 
#ifndef RING_DEFINED
#define RING_DEFINED

#include <stdint.h>

#include "dcc.h"

#define T                  Ring_T
#define RING_TYPE_INT      0
#define RING_TYPE_CHAR     1
#define RING_TYPE_PACKET   2

union Ring_data
{
    int          i;
    char         c;
    DCC_packet_T p;
};

typedef struct T *T;
struct T
{
    union Ring_data *buf;
    int   type;
    int   count;
    int   start;
    int   size;
};

extern T     Ring_create(int ring_type, int size);
extern void  Ring_destroy(T ring);
extern void  Ring_push(T ring, union Ring_data data);
extern union Ring_data Ring_pop(T ring);
extern void  Ring_reset(T ring);

#undef T
#endif
