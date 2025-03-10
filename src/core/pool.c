#include "core/pool.h"
#include "core/log.h"

Pool pool_make(isize count, isize block_size, isize block_alignment) {
  Pool pool = {
      .arena = arena_make(count * block_size, ARENA_FLAG_DEFAULTS),
      .free_block = NULL,
      .block_size = ALIGN_ROUND_UP(block_size, block_alignment),
      .block_last_occupied = 0,
  };

  ASSERT(block_size >= sizeof(Pool_Block), "Requested pool block size is too small");

  return pool;
}

void pool_free(Pool *pool) {
  arena_free(&pool->arena);
  ZERO_STRUCT(pool);
}

void *pool_alloc(Pool *pool) {
  void *ptr = NULL;

  // We have a free block! Take that open spot first
  if (pool->free_block != NULL) {
    ptr = pool->free_block;
    pool->free_block = pool->free_block->next;
  } else {
    // Don't have a free block, add to the end alignment is 1 since we can just pack these like an
    // array, as well we can use the arena logic to resize and check capacity and such
    ptr = arena_alloc(&pool->arena, pool->block_size, 1);
    pool->block_last_occupied++;
  }

  return ptr;
}

void pool_pop(Pool *pool, void *ptr) {
  void *pool_base = pool->arena.base;
  void *pool_filled = pool->arena.base + pool->arena.next_offset;
  ASSERT(ptr >= pool_base && ptr <= pool_filled, "Tried to pop pool element outside of pool");

  ZERO_SIZE(ptr, pool->block_size);

  // TODO(ss): Seems to be working, but needs more robust verification
  // Checks if the one we are freeing is the very last one
  {
    isize last_block_offset = (pool->block_last_occupied - 1) * pool->block_size;
    void *last_block_ptr = (void *)(last_block_offset + (isize)pool->arena.base);
    if (last_block_ptr == ptr)
      pool->block_last_occupied--;
  }

  // Add this to the start of linked list
  Pool_Block *new_free = (Pool_Block *)ptr;
  new_free->next = pool->free_block;

  pool->free_block = new_free;
}

void *pool_as_array(Pool *pool, u32 *out_last_index) {
  if (out_last_index != NULL) {
    *out_last_index = pool->block_last_occupied;
  }

  return pool->arena.base;
}
