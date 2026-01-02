# Giga Hash Map (C89)

Giga Hash Map is a high-performance, arena-driven hash map written in
strict C89.

It is **allocator-agnostic by design** and pairs naturally with
**giga-arena**, but does not depend on it directly. Instead, it consumes
a minimal allocator ABI contract, allowing the hash map to operate over
any compatible arena or custom allocator.

This design mirrors how serious systems (compilers, engines, kernels)
separate **allocation policy** from **data structure semantics**.

---

## Goals

- High-performance hash map for long-lived data
- Deterministic memory ownership
- Arena-driven allocation
- No per-entry malloc / free usage
- Strict C89 compatibility
- Single translation unit
- Auditable, minimal implementation
- Suitable for compilers, analyzers, and systems tooling

Non-goals:

- Being a drop-in replacement for stdlib hash tables
- Supporting fine-grained deletes
- Optimizing for short-lived, highly dynamic workloads

---

## Design Philosophy

The core principle is:

Memory policy is external.  
Data structure semantics are internal.

Giga Hash Map does **not** decide:

- how memory is reserved
- how memory is committed
- how memory is freed
- which OS primitives are used

It only decides:

- how keys and values are stored
- how hashing and probing work
- how collisions are resolved
- how growth and rebuilds occur

This separation keeps the hash map simple, portable, and reusable, while
allowing sophisticated allocation strategies underneath.

---

## Arena-Driven Design

All internal storage is allocated from an external arena:

- entry arrays
- buckets
- rebuilds during growth

There are no per-entry frees.
Destruction is implicit when the arena is reset or destroyed.

This model is ideal for:

- symbol tables
- interning tables
- compiler passes
- static analysis
- security tooling
- phase-oriented systems

---

## Hash Map Design

The current implementation uses:

- Open addressing
- Quadratic probing
- Explicit occupancy flags
- Power-of-two capacity
- Load-factor guarding
- Fast integer hashing (SplitMix-style finalizer)

The implementation is intentionally conservative and auditable, favoring
predictable performance over clever tricks.

---

## ABI Contract with Giga Arena

Giga Hash Map does **not include** giga-arena.

Instead, it relies on a **minimal allocator ABI** that any arena-like
allocator can satisfy.

The required interface is intentionally small:

    typedef struct Arena Arena;

    void *arena_alloc(Arena *arena, size_t size);

That is the entire dependency surface.

No headers are shared.
No implementation details are assumed.
No global state is required.

This means:

- Giga Hash Map can be used with giga-arena
- Giga Hash Map can be used with a custom arena
- Allocation strategy can be swapped without changing hash map code
- The hash map remains portable and testable

The allocator is treated as a **service**, not a dependency.

---

## Why ABI Instead of a Hard Dependency

Using an ABI contract instead of a concrete dependency provides:

- Zero coupling between repositories
- Cleaner reasoning about responsibilities
- Easier auditing
- Easier reuse in other systems
- Long-term stability of the interface

This is the same pattern used in:

- compilers
- game engines
- kernels
- virtual machines

---

## File Layout

    .
    ├── main.c
    └── ReadMe.md

The entire implementation and benchmark live in a single C file.

---

## Benchmark

The repository includes a built-in benchmark that compares:

- an arena-backed hash map
- a malloc-backed hash map

Both use **identical hashing, probing, and table logic**.  
The only difference is the allocation strategy.

Benchmark parameters:

- Inserts: 1,000,000
- Capacity: 4,000,000
- Key type: 64-bit integers
- Open addressing with quadratic probing

Example results (machine dependent):

    ARENA HASHMAP
      inserts/sec : ~72 million

    MALLOC HASHMAP
      inserts/sec : ~49 million

This represents a **~1.47× throughput improvement** when using an
arena-backed allocation strategy.

The gap is intentionally modest and realistic: hashing and probing
dominate runtime once the algorithm is efficient. The arena still wins
by reducing allocator overhead, metadata traffic, and cache disruption.

---

## Interpreting the Results

The benchmark is intentionally honest:

- No toy loops
- No artificial allocation churn
- No optimizer tricks
- Same algorithm, same code paths

As the hash map becomes more efficient, allocator impact naturally
shrinks. The fact that arena-backed allocation still outperforms
malloc-backed allocation under these conditions is the key result.

---

## Expected Usage Pattern

A typical usage flow looks like this:

    Arena arena;
    arena_init(&arena, ...);

    HashMap map;
    hashmap_init(&map, &arena);

    hashmap_put(&map, key, value);
    hashmap_get(&map, key);

    arena_reset(&arena);   /* implicit destruction */
    arena_destroy(&arena);

The hash map never frees memory itself.

---

## Growth and Rebuild Semantics

Because memory is arena-backed:

- Growth allocates new storage
- Old storage is abandoned
- No in-place reallocation is required
- Rebuild cost is explicit and predictable

This trades memory reuse for:

- simplicity
- performance
- determinism

Which is exactly what arena-based systems want.

---

## When to Use Giga Hash Map

Use this hash map when:

- Data lives for a known phase or lifetime
- You already control memory ownership
- You want predictable performance
- You want to avoid allocator overhead
- You are building compilers or analyzers

---

## When NOT to Use It

Do not use this hash map when:

- You need fine-grained deletes
- You rely on malloc/free lifetimes
- You need highly dynamic churn
- You want a general-purpose container

This is a **systems tool**, not a stdlib replacement.

---

## Relationship to Giga Arena

Giga Hash Map is designed to work naturally with giga-arena, but remains
fully independent.

Think of giga-arena as the **memory substrate**, and giga-hashmap as a
**consumer of that substrate**.

Each can evolve independently.
Each remains understandable in isolation.

---

## Status

- Implementation complete
- ABI contract stable
- Benchmark included
- Arena-backed and malloc-backed paths verified
- Strict C89

---

## License

Public domain / Unlicense / MIT — your choice.

This code is intended to be studied, adapted, and embedded into
serious systems.
