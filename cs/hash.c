/**
 * @file hash.c
 * @brief Implements the hash table interface.
 * @author Mikey Austin
 * @date 2012-2013
 */
 
#include <stdlib.h>

#include "hash.h"

#define T Hash_T
#define E Hash_entry
#define K Hash_key

/**
 * Lookup a key in the hash table and return an entry pointer. This function
 * contains the logic for linear probing conflict resolution.
 *
 * An entry with a NULL value indicates that the entry is not being used.
 */
static struct E *Hash_find_entry(T hash, union K *key);

extern T
Hash_create(int size, int (*lookup)(union K *key),
    int (*cmp)(union K *a, union K *b), void (*destroy)(struct E *entry))
{
    int i;
    T new;

    new = (T) malloc(sizeof(*new));
    if(!new)
        return NULL;

    new->size    = size;
    new->lookup  = lookup;
    new->cmp     = cmp;
    new->destroy = destroy;

    new->entries = (struct E*) malloc(sizeof(*(new->entries)) * size);
    if(!new->entries)
    {
        free(new);
        return NULL;
    }

    /* Initialise the entries to NULL & indexes to -1. */
    for(i=0; i < new->size; i++)
    {
        new->entries[i].v = NULL;
    }

    return new;
}

extern void
Hash_destroy(T hash)
{
    int i;

    for(i=0; i < hash->size; i++)
        hash->destroy(&(hash->entries[i]));

    free(hash->entries);
    free(hash);
}

extern void
Hash_reset(T hash)
{
    int i;

    for(i=0; i < hash->size; i++)
    {
        hash->destroy(&(hash->entries[i]));
        hash->entries[i].v = NULL;
    }
}

extern void
Hash_insert(T hash, union K key, void *value)
{
    struct E *entry;
    
    entry = Hash_find_entry(hash, &key);
    if(entry->v != NULL)
    {
        /* An entry exists, clear contents before overwriting. */
        hash->destroy(entry);
    }

    /* Setup the new entry. */
    entry->k = key;
    entry->v = value;
}

extern void
*Hash_get(T hash, union K *key)
{
    struct E *entry;
    entry = Hash_find_entry(hash, key);
    return entry->v;
}

static struct E
*Hash_find_entry(T hash, union K *key)
{
    int i, j;
    struct E *curr;

    i = j = (hash->lookup(key) % hash->size);
    do
    {
        curr = hash->entries + i;
        if((hash->cmp(key, &(curr->k)) == 0) || (curr->v == NULL))
            break;

        i = ((i + 1) % hash->size);
    }
    while(i != j);

    return curr;
}

#undef T
#undef E
#undef K
