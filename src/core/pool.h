#ifndef POOL_H
#define POOL_H

#include "core/arena.h"

typedef struct Pool_Block Pool_Block;
struct Pool_Block {
    Pool_Block *next;
};

// Backed by an arena, this may not be great, since using arena functions we may be going through a
// lot of logic that isn't neseccary for this data structure? Need to think more.
typedef struct Pool Pool;
struct Pool {
    Arena arena;
    Pool_Block *free_block;
    u64 block_size;
    u64 block_capacity;
    u64 block_last_index; // We need to keep track of the last occupied index if we want to make
                          // sure to loop through all elements
};

Pool pool_create(u64 count, u64 block_size, u64 block_alignment);
void pool_free(Pool *pool);

void *pool_alloc(Pool *pool);
void pool_pop(Pool *pool, void *ptr);

// Cast this as your underlying type to access it like an array!
// Use pool->blocks_occupied
void *pool_as_array(Pool *pool);

#define pool_create_type(c, T) pool_create(c, sizeof(T), alignof(T))

#endif // POOL_H
