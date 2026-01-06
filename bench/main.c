#include <stdio.h>
#include <giga/arena.h>
#include <giga/hashmap.h>

int main(void)
{
    Arena arena;
    HashMap *map;

    if (arena_init(&arena, 1024u * 1024u, 64u * 1024u) != 0) {
        printf("arena_init failed\n");
        return 1;
    }

    map = hashmap_create(&arena, 1024);
    if (!map) {
        printf("hashmap_create failed\n");
        return 1;
    }

    /* benchmark loop goes here */

    /* no hashmap_destroy needed if arena-backed, but fine if you implement it */
    arena_destroy(&arena);
    return 0;
}
