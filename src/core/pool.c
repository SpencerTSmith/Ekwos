#include "core/pool.h"
#include "core/log.h"
#include <assert.h>

Pool pool_create(u64 count, u64 block_size, u64 block_alignment) {
  Pool pool = {
      .arena = arena_create(count * block_size, ARENA_FLAG_DEFAULTS),
      .free_block = NULL,
      .block_size = ALIGN_ROUND_UP(block_size, block_alignment),
      .block_last_index = 0,
      .block_capacity = count,
  };

  assert(block_size >= sizeof(Pool_Block) && "Requested pool block size is too small");

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
  } else { // Don't have a free block, add to the end
    // alignment is 1 since we can just pack these like an array,
    // as well we can use the arena logic to resize and check capacity and such
    ptr = arena_alloc(&pool->arena, pool->block_size, 1);
    pool->block_last_index++;
  }

  return ptr;
}

void pool_pop(Pool *pool, void *ptr) {
  void *pool_base = pool->arena.base_ptr;
  void *pool_filled = pool->arena.base_ptr + pool->arena.offset;
  if (ptr < pool_base || ptr >= pool_filled) {
    LOG_ERROR("Tried to pop pool element outside of pool");
    return;
  }

  ZERO_SIZE(ptr, pool->block_size);

  // Add this to the start of linked list
  Pool_Block *new_free = ptr;
  new_free->next = pool->free_block;

  pool->free_block = new_free;
}

void *pool_as_array(Pool *pool, u32 *out_last_index) {
  if (out_last_index != NULL) {
    *out_last_index = pool->block_last_index;
  }
  return pool->arena.base_ptr;
}
