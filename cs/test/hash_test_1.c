#include <stdio.h>
#include "hash.h"

int lookup(union Hash_key *key);
int cmp(union Hash_key *a, union Hash_key *b);
void destroy(struct Hash_entry *entry);
void dump(Hash_T h);

int
main(void)
{
    Hash_T h;
    union Hash_key k1, k2, k3, k4, k5;
    char *s1 = "string 1", *s2 = "string 2", *s3 = "string 3", *s4 = "string 4", *s5 = "string 5";
    char *ow1 = "overwritten 1", *ow2 = "overwritten 2";
    char *ac = "after clear";
    char *t1, *t2, *t3, *t4, *t5;

    // Prepare hash entries.
    k1.i = 32;
    k2.i = 10;
    k3.i = 25;
    k4.i = 20;
    k5.i = 30;

    h = Hash_create(10, lookup, cmp, destroy);
    Hash_insert(h, k1, (void*) s1);
    Hash_insert(h, k2, (void*) s2);
    Hash_insert(h, k3, (void*) s3);
    Hash_insert(h, k4, (void*) s4);
    Hash_insert(h, k5, (void*) s5);

    t1 = Hash_get(h, &k1);
    t2 = Hash_get(h, &k2);
    t3 = Hash_get(h, &k3);
    t4 = Hash_get(h, &k4);
    t5 = Hash_get(h, &k5);

    printf("Key: %d, \tValue: %s\n", k1.i, t1);
    printf("Key: %d, \tValue: %s\n", k2.i, t2);
    printf("Key: %d, \tValue: %s\n", k3.i, t3);
    printf("Key: %d, \tValue: %s\n", k4.i, t4);
    printf("Key: %d, \tValue: %s\n", k5.i, t5);

    dump(h);

    printf("\nOverwriting %d...\n", k1.i);
    Hash_insert(h, k1, ow1);
    t1 = Hash_get(h, &k1);
    printf("Key: %d, \tValue: %s\n", k1.i, t1);

    dump(h);

    printf("\nOverwriting %d...\n", k4.i);
    Hash_insert(h, k4, ow2);
    t1 = Hash_get(h, &k4);
    printf("Key: %d, \tValue: %s\n", k4.i, t1);

    dump(h);

    printf("\nClearing...\n");
    Hash_clear(h);

    printf("\nAdding %d...\n", k4.i);
    Hash_insert(h, k4, ac);
    t1 = Hash_get(h, &k4);
    printf("Key: %d, \tValue: %s\n", k4.i, t1);

    dump(h);

    return 0;
}

void
dump(Hash_T h)
{
    int i;
    printf("\nDumping table...\n");
    for(i=0; i < h->size; i++)
    {
        printf("i: %d, ", i);
        if(h->entries[i].v == NULL)
        {
            printf("EMPTY\n");
        }
        else
        {
            printf("Key: %d, \tValue: %s\n", h->entries[i].k.i, (char*) h->entries[i].v);
        }
    }
}

int
lookup(union Hash_key *key)
{
    return key->i;
}

int
cmp(union Hash_key *a, union Hash_key *b)
{
    return (a->i == b->i ? 0 : 1);
}

void
destroy(struct Hash_entry *entry)
{
    entry->v = NULL;
}
