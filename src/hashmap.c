#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include <giga/hashmap.h>
#include <giga/arena.h>

/* ============================================================
 * Internal structures
 * ============================================================ */

/*
 * C89: assume LP64 / LLP64.
 * unsigned long must be >= 32 bits.
 * We REQUIRE 64-bit.
 */
typedef unsigned long hm_u64;

enum {
    HM_U64_IS_64_BIT = 1 / (sizeof(hm_u64) == 8)
};

typedef struct Entry {
    const char   *key;
    void         *value;
    hm_u64        hash;
    unsigned char used;       /* slot has ever been used */
    unsigned char tombstone;  /* entry was removed */
} Entry;

struct HashMap {
    Entry        *entries;
    size_t        capacity;
    size_t        count;
    struct Arena *arena;
};

/* ============================================================
 * Utilities
 * ============================================================ */

/* FNV-1a 64-bit (C89-safe constants) */
static hm_u64 hash_str(const char *s)
{
    hm_u64 h;

    h = (hm_u64)1469598103934665603UL;

    while (*s) {
        h ^= (unsigned char)*s++;
        h *= (hm_u64)1099511628211UL;
    }

    return h;
}

static size_t next_pow2(size_t x)
{
    size_t p = 1;
    while (p < x) {
        p <<= 1;
    }
    return p;
}

/* ============================================================
 * Lifecycle
 * ============================================================ */

HashMap *hashmap_create(struct Arena *arena, size_t capacity)
{
    HashMap *m;
    size_t   i;

    if (!arena) {
        return 0;
    }

    m = (HashMap *)arena_alloc(arena, sizeof(HashMap));
    if (!m) {
        return 0;
    }

    m->capacity = capacity ? next_pow2(capacity) : 16;
    m->count    = 0;
    m->arena    = arena;

    m->entries = (Entry *)arena_alloc(
        arena,
        m->capacity * sizeof(Entry)
    );

    if (!m->entries) {
        return 0;
    }

    for (i = 0; i < m->capacity; ++i) {
        m->entries[i].used      = 0;
        m->entries[i].tombstone = 0;
        m->entries[i].key       = 0;
        m->entries[i].value     = 0;
        m->entries[i].hash      = 0;
    }

    return m;
}

HashMap *hashmap_clone(HashMap *src, struct Arena *arena)
{
    HashMap *m;
    size_t i;

    if (!src || !arena) {
        return 0;
    }

    m = hashmap_create(arena, src->capacity);
    if (!m) {
        return 0;
    }

    /* STRUCTURAL COPY — NOT rehashing */
    for (i = 0; i < src->capacity; ++i) {
        m->entries[i] = src->entries[i];
    }

    m->count = src->count;
    return m;
}

/* ============================================================
 * Operations
 * ============================================================ */

void *hashmap_get(HashMap *map, const char *key)
{
    hm_u64 hash;
    size_t i;
    size_t mask;
    size_t step;

    if (!map || !key) {
        return 0;
    }

    hash = hash_str(key);
    mask = map->capacity - 1;
    i    = (size_t)hash & mask;
    step = 0;

    for (;;) {
        Entry *e = &map->entries[i];

        if (!e->used) {
            return 0; /* not found */
        }

        if (e->hash == hash && strcmp(e->key, key) == 0) {
            return e->value;
        }

        i = (i + ++step) & mask;
    }
}


int hashmap_put(HashMap *map, const char *key, void *value)
{
    hm_u64 hash;
    size_t i, mask, step;
    Entry *first_tombstone = 0;

    if (!map || !key) {
        return 0;
    }

    hash = hash_str(key);
    mask = map->capacity - 1;
    i    = (size_t)hash & mask;
    step = 0;

    for (;;) {
        Entry *e = &map->entries[i];

        if (!e->used) {
            if (first_tombstone) {
                e = first_tombstone;
            }

            e->used      = 1;
            e->tombstone = 0;
            e->hash      = hash;
            e->key       = key;
            e->value     = value;
            map->count++;
            return 1;
        }

        if (e->tombstone && !first_tombstone) {
            first_tombstone = e;
        } else if (!e->tombstone &&
                   e->hash == hash &&
                   strcmp(e->key, key) == 0) {
            e->value = value;
            return 1;
        }

        i = (i + ++step) & mask;
    }
}

int hashmap_remove(HashMap *map, const char *key)
{
    hm_u64 hash;
    size_t i, mask, step;

    if (!map || !key) {
        return 0;
    }

    hash = hash_str(key);
    mask = map->capacity - 1;
    i    = (size_t)hash & mask;
    step = 0;

    for (;;) {
        Entry *e = &map->entries[i];

        /* Never-used slot => key not present */
        if (!e->used && !e->tombstone) {
            return 0;
        }

        if (e->used &&
            !e->tombstone &&
            e->hash == hash &&
            strcmp(e->key, key) == 0) {

            e->tombstone = 1; /* preserve probe chain */
            map->count--;
            return 1;
        }

        i = (i + ++step) & mask;
    }
}
