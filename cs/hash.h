/**
 * @file hash.h
 * @brief Defines an abstract hash table data structure.
 * @author Mikey Austin
 * @date 2012-2013
 */
 
#ifndef HASH_DEFINED
#define HASH_DEFINED

#define T Hash_T
#define E Hash_entry
#define K Hash_key

/**
 * A union to hold key data.
 */
union K
{
    int  i;
};

/**
 * A struct to contain a hash entry's key and value pair.
 */
struct E
{
    union K k;    /**< Hash entry key */
    void    *v;   /**< Hash entry value */
};

/**
 * The main hash table structure.
 */
typedef struct T *T;
struct T
{
    int size;
    int (*lookup)(union K *key);
    int (*cmp)(union K *a, union K *b);
    void (*destroy)(struct E *entry);
    struct E *entries;
};

/**
 * Create a new hash table.
 */
extern T Hash_create(int size,
                     int (*lookup)(union K *key),
                     int (*cmp)(union K *a, union K *b),
                     void (*destroy)(struct E *entry));

/**
 * Destroy a hash table, freeing all elements
 */
extern void Hash_destroy(T hash);

/**
 * Clear out all entries in the table.
 */
extern void Hash_reset(T hash);

/**
 * Insert a new element into the hash table.
 */
extern void Hash_insert(T hash, union K key, void *value);

/**
 * Fetch an element from the hash table by the specified key.
 */
extern void *Hash_get(T hash, union K *key);

#undef T
#undef E
#undef K
#endif
