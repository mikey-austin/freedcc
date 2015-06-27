/**
 * @file ring.c
 * @brief Defines a ring buffer data structure implementation.
 * @author Mikey Austin
 * @date 2010-2011
 *
 * This module defines a generic ring buffer.
 */

#include <stdlib.h>

#include "ring.h"

#define T       Ring_T
 
extern void
Ring_push(T ring, union Ring_data data)
{
    ring->buf[(ring->start + ring->count) % ring->size] = data;
    ring->count++;
}

extern union Ring_data
Ring_pop(T ring)
{
    union Ring_data popped;

    popped = ring->buf[ring->start];
    ring->start = (ring->start + 1) % ring->size;
    ring->count--;

    return popped;
}

extern void
Ring_reset(T ring)
{
    ring->count = 0;
    ring->start = 0;
}

extern T
Ring_create(int ring_type, int size)
{
    T ring;

    ring = (T) malloc(sizeof(*ring));
    if(ring == NULL)
    {
        return NULL;
    }

    ring->buf = malloc(sizeof(*(ring->buf)) * size);
    if(ring->buf == NULL)
    {
        free(ring);
        return NULL;
    }

    ring->type = ring_type;
    ring->size = size;
    Ring_reset(ring);

    return ring;
}

extern void
Ring_destroy(T ring)
{
    if(ring && ring->buf)
        free(ring->buf);

    ring->buf = NULL;

    if(ring)
        free(ring);

    ring = NULL;
}
