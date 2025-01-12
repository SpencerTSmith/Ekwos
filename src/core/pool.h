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
};

Pool pool_create(u64 count, u64 block_size);
void pool_free(Pool *pool);

void *pool_alloc(Pool *pool, u64 count);
void pool_pop(Pool *pool, void *ptr);

#define pool_type_create(c, T) pool_create(c, sizeof(block_size))

#endif // POOL_H
