#ifndef POOL_H
#define POOL_H

#include "core/arena.h"

typedef struct Pool_Block Pool_Block;
struct Pool_Block {
  Pool_Block *next;
};

// NOTE(ss): Mostly plan on using this as an array with a free list...
// not nessecarily like a general purpose pool allocator

// Backed by an arena, this may not be great, since using arena functions we may be going through a
// lot of logic that isn't neseccary for this data structure? Need to think more.
typedef struct Pool Pool;
struct Pool {
  Arena arena;
  Pool_Block *free_block;
  isize block_size;

  // We need to keep track of the last occupied index if we want to make sure to loop through all
  // elements in a pool, but do we want do do this??
  u32 block_last_index;
};

Pool pool_make(u64 block_count, u64 block_size, u64 block_alignment);
void pool_free(Pool *pool);

void *pool_alloc(Pool *pool);
void pool_pop(Pool *pool, void *ptr);

// Cast this as your underlying type to access it like an array!
// Use pool->blocks_occupied
void *pool_as_array(Pool *pool, u32 *out_last_index);

#define pool_make_type(c, T) pool_make(c, sizeof(T), alignof(T))

#endif // POOL_H
