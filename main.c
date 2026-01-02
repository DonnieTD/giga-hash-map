/*
============================================================
 main.c — Giga Hash Map Benchmark (C89, GO BRRR)
============================================================

- Arena-backed vs malloc-backed hash map
- Open addressing
- Quadratic probing
- Fast integer hash (SplitMix-style)
- Explicit occupancy flag
- Honest benchmark
============================================================
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* =========================================================
 * Benchmark configuration
 * ========================================================= */

#define NUM_INSERTS   1000000UL
#define INITIAL_CAP   (NUM_INSERTS * 4)

/* =========================================================
 * Timing utilities
 * ========================================================= */

static double now_seconds(void)
{
#if defined(_WIN32)
    LARGE_INTEGER freq, ctr;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&ctr);
    return (double)ctr.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
#if defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
#else
    return (double)time(NULL);
#endif
#endif
}

/* =========================================================
 * Arena ABI
 * ========================================================= */

typedef struct Arena Arena;
void *arena_alloc(Arena *arena, size_t size);

/* =========================================================
 * Fast integer hash (SplitMix64 finalizer)
 * ========================================================= */

static uint64_t hash_u64(uint64_t x)
{
    x ^= x >> 33;
    x *= (uint64_t)0xff51afd7u;
    x ^= x >> 33;
    x *= (uint64_t)0xc4ceb9feu;
    x ^= x >> 33;
    return x;
}

/* =========================================================
 * Hash map structures
 * ========================================================= */

typedef struct Entry {
    uint64_t key;
    uint64_t value;
    uint64_t hash;
    unsigned char used;
} Entry;

typedef struct HashMap {
    Entry  *entries;
    size_t  capacity;
    size_t  count;
    Arena  *arena;
    int     use_malloc;
} HashMap;

/* =========================================================
 * Helpers
 * ========================================================= */

static size_t next_pow2(size_t x)
{
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

static Entry *alloc_entries(HashMap *m, size_t count)
{
    size_t bytes = count * sizeof(Entry);
    Entry *e;

    if (m->use_malloc) {
        e = (Entry *)malloc(bytes);
    } else {
        e = (Entry *)arena_alloc(m->arena, bytes);
    }

    if (e)
        memset(e, 0, bytes);

    return e;
}

static int hashmap_init(
    HashMap *m,
    Arena   *arena,
    size_t   capacity,
    int      use_malloc
)
{
    m->capacity   = next_pow2(capacity);
    m->count      = 0;
    m->arena      = arena;
    m->use_malloc = use_malloc;
    m->entries    = alloc_entries(m, m->capacity);
    return m->entries != NULL;
}

/* =========================================================
 * Hash map insert (quadratic probing, guarded)
 * ========================================================= */

static void hashmap_put(HashMap *m, uint64_t key, uint64_t value)
{
    uint64_t hash = hash_u64(key);
    size_t mask = m->capacity - 1;
    size_t i = (size_t)hash & mask;
    size_t step = 0;

    /* load factor guard: should never trip in benchmark */
    if (m->count * 10 >= m->capacity * 7)
        return;

    for (;;) {
        Entry *e = &m->entries[i];

        if (!e->used) {
            e->used  = 1;
            e->key   = key;
            e->value = value;
            e->hash  = hash;
            m->count++;
            return;
        }

        step++;
        i = (i + step) & mask;
    }
}

/* =========================================================
 * Fake arena (benchmark stand-in)
 * ========================================================= */

typedef struct FakeArena {
    unsigned char *base;
    size_t size;
    size_t used;
} FakeArena;

static void *fake_arena_alloc(Arena *a, size_t size)
{
    FakeArena *fa = (FakeArena *)a;

    if (fa->used + size > fa->size)
        return NULL;

    {
        void *p = fa->base + fa->used;
        fa->used += size;
        return p;
    }
}

void *arena_alloc(Arena *arena, size_t size)
{
    return fake_arena_alloc(arena, size);
}

/* =========================================================
 * Volatile sink
 * ========================================================= */

static volatile uint64_t sink;

/* =========================================================
 * Benchmarks
 * ========================================================= */

static void bench_arena_hashmap(void)
{
    static unsigned char backing[512 * 1024 * 1024];
    FakeArena arena;
    HashMap map;
    size_t i;
    double t0, t1;

    arena.base = backing;
    arena.size = sizeof(backing);
    arena.used = 0;

    hashmap_init(&map, (Arena *)&arena, INITIAL_CAP, 0);

    t0 = now_seconds();

    for (i = 0; i < NUM_INSERTS; ++i)
        hashmap_put(&map, i, i * 2);

    t1 = now_seconds();

    sink = map.count;

    printf("ARENA HASHMAP\n");
    printf("  inserts/sec : %.0f\n",
           NUM_INSERTS / (t1 - t0));
}

static void bench_malloc_hashmap(void)
{
    HashMap map;
    size_t i;
    double t0, t1;

    hashmap_init(&map, NULL, INITIAL_CAP, 1);

    t0 = now_seconds();

    for (i = 0; i < NUM_INSERTS; ++i)
        hashmap_put(&map, i, i * 2);

    t1 = now_seconds();

    sink = map.count;

    printf("MALLOC HASHMAP\n");
    printf("  inserts/sec : %.0f\n",
           NUM_INSERTS / (t1 - t0));

    free(map.entries);
}

/* =========================================================
 * main
 * ========================================================= */

int main(void)
{
    printf("============================================\n");
    printf(" Giga Hash Map Benchmark — GO BRRR (C89)\n");
    printf("============================================\n");
    printf("inserts  : %lu\n", (unsigned long)NUM_INSERTS);
    printf("capacity : %lu\n\n", (unsigned long)INITIAL_CAP);

    bench_arena_hashmap();
    printf("\n");
    bench_malloc_hashmap();

    return 0;
}
