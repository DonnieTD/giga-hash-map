#ifndef GIGA_HASHMAP_H
#define GIGA_HASHMAP_H

#include <stddef.h> /* size_t */

/*
============================================================
 giga-hash-map — Public API
============================================================

 Open-addressing hash map with quadratic probing.

 Design constraints:
 - C89
 - Arena-backed allocation
 - No per-entry free
 - Explicit ownership
 - Long-lived, stable ABI

 This module depends on giga-arena for memory allocation.
============================================================
*/

/*
------------------------------------------------------------
 Forward declarations
------------------------------------------------------------
*/

/*
 * Arena is defined in giga-arena.
 * The hashmap borrows the arena and never owns it.
 */
struct Arena;

/*
 * HashMap is opaque to users.
 * Internal layout is not part of the public contract.
 */
typedef struct HashMap HashMap;

/*
------------------------------------------------------------
 Lifecycle
------------------------------------------------------------
*/

/*
 * hashmap_create
 *
 * Create a new hashmap backed by the given arena.
 *
 * All internal memory (the HashMap object and entry table)
 * is allocated from the arena.
 *
 * Parameters:
 *   arena    - Arena used for all allocations (must not be NULL)
 *   capacity - Initial entry capacity (rounded up to power-of-two)
 *
 * Ownership:
 *   - The caller owns the arena
 *   - The arena must outlive the hashmap
 *
 * Returns:
 *   Pointer to a new HashMap on success
 *   NULL on allocation failure
 */
HashMap *hashmap_create(struct Arena *arena, size_t capacity);

/*
 * hashmap_clone
 *
 * Clone an existing hashmap into a new arena.
 *
 * This performs a shallow copy:
 *   - Keys are copied by pointer
 *   - Values are copied by pointer
 *
 * No key or value memory is duplicated.
 *
 * Parameters:
 *   src   - Source hashmap
 *   arena - Arena for the new hashmap
 *
 * Returns:
 *   Pointer to a new HashMap on success
 *   NULL on failure
 */
HashMap *hashmap_clone(HashMap *src, struct Arena *arena);

/*
------------------------------------------------------------
 Operations
------------------------------------------------------------
*/

/*
 * hashmap_get
 *
 * Lookup a value by key.
 *
 * Parameters:
 *   map - HashMap instance
 *   key - NUL-terminated string key
 *
 * Returns:
 *   Value pointer if the key exists
 *   NULL if the key is not present
 */
void *hashmap_get(HashMap *map, const char *key);

/*
 * hashmap_put
 *
 * Insert or update a key/value pair.
 *
 * Keys are NOT copied.
 * The caller must ensure the key string remains valid
 * for the lifetime of the hashmap.
 *
 * Parameters:
 *   map   - HashMap instance
 *   key   - NUL-terminated string key
 *   value - Value pointer
 *
 * Returns:
 *   1 on success
 *   0 on failure
 */
int hashmap_put(HashMap *map, const char *key, void *value);

/*
 * hashmap_remove
 *
 * Remove a key from the hashmap.
 *
 * This is a logical removal only.
 * Memory is NOT reclaimed.
 *
 * Parameters:
 *   map - HashMap instance
 *   key - NUL-terminated string key
 *
 * Returns:
 *   1 if the key was removed
 *   0 if the key was not found
 */
int hashmap_remove(HashMap *map, const char *key);

/*
------------------------------------------------------------
 Introspection
------------------------------------------------------------
*/

/*
 * hashmap_count
 *
 * Return the number of active entries.
 */
size_t hashmap_count(HashMap *map);

/*
 * hashmap_capacity
 *
 * Return the total entry capacity.
 */
size_t hashmap_capacity(HashMap *map);

#endif /* GIGA_HASHMAP_H */
