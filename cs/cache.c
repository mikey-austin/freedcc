/**
 * @file cache.c
 * @brief Implements the cache interface.
 * @author Mikey Austin
 * @date 2012-2013
 */

#include <stdlib.h>

#include "ring.h"
#include "hash.h"
#include "cache.h"

#define CACHE_ADDR_SIZE 20

/**
 * Ring containing the active loco addresses.
 */
static Ring_T Cache_addresses;

/**
 * Hash table containing the packets for each loco.
 */
static Hash_T Cache_packets;

/**
 * Private callback functions for use in the packet hash.
 */
int Cache_hash_lookup(union Hash_key *key);
int Cache_hash_key_cmp(union Hash_key *a, union Hash_key *b);
void Cache_hash_entry_destroy(struct Hash_entry *entry);

extern void
Cache_module_init(void)
{
    /* Initialise address ring. */
    Cache_addresses = Ring_create(RING_TYPE_INT, CACHE_ADDR_SIZE);

    /* Initialise packet hash. */
    Cache_packets = Hash_create(CACHE_ADDR_SIZE, Cache_hash_lookup,
        Cache_hash_key_cmp, Cache_hash_entry_destroy);
}

extern void
Cache_clear(void)
{
    Ring_reset(Cache_addresses);
    Hash_reset(Cache_packets);
}

extern void
Cache_update(DCC_packet_T packet)
{
    union Ring_data address;
    union Hash_key key;

    /* Extract the address from the packet. */
    address.i = key.i = (int) DCC_get_address(packet);

    if(Hash_get(Cache_packets, &key) == NULL)
    {
        /*
         * This loco has not been seen yet, so store address.
         */
        Ring_push(Cache_addresses, address);
    }

    Hash_insert(Cache_packets, key, (void *) packet);
}

extern DCC_packet_T
Cache_get_next_packet(void)
{
    union Ring_data next;
    union Hash_key key;
    DCC_packet_T packet;

    if(Cache_addresses->count <= 0)
    {
        /* Cache is empty. */
        return NULL;
    }

    next = Ring_pop(Cache_addresses);
    key.i = next.i;
    packet = (DCC_packet_T) Hash_get(Cache_packets, &key);

    /* Push popped address to the end of the queue. */
    Ring_push(Cache_addresses, next);

    return packet;
}

extern
DCC_packet_T Cache_get(int address)
{
    union Hash_key key;
    DCC_packet_T packet;

    key.i = address;
    packet = (DCC_packet_T) Hash_get(Cache_packets, &key);

    return packet;
}

int
Cache_hash_lookup(union Hash_key *key)
{
    /* 
     * Extremely simple hash function. We don't need to worry about
     * modding the result with the table size here.
     */
    return key->i;
}

int
Cache_hash_key_cmp(union Hash_key *a, union Hash_key *b)
{
    /* Keys are integers in this hash. */
    return (a->i == b->i ? 0 : 1);
}

void
Cache_hash_entry_destroy(struct Hash_entry *entry)
{
    if(entry->v != NULL)
    {
        DCC_packet_destroy((DCC_packet_T) entry->v);
        entry->v = NULL;
    }
}

extern int 
Cache_report_current_size(void)
{
    return Cache_addresses->count;
}

extern int
Cache_report_total_size(void)
{
    return CACHE_ADDR_SIZE;
}
