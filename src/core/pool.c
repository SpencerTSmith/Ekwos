#include "core/pool.h"
#include "core/log.h"

Pool pool_create(u64 count, u64 block_size) {
    Pool pool = {
        .block_size = block_size,
        .free_block = NULL,
        .arena = arena_create(count * block_size, ARENA_FLAG_DEFAULTS),
    };

    return pool;
}

void *pool_alloc(Pool *pool, u64 count) {
    void *ptr = NULL;

    // We don't
    if (pool->free_block == NULL) {
        // alignment is 1 since we can just pack these like an array,
        // as well we can use the arena logic to resize and check capacity and such
        ptr = arena_alloc(&pool->arena, pool->block_size, 1);
    } else {
        ptr = pool->free_block;
        pool->free_block = pool->free_block->next;
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
    Pool_Block *new_free = ptr;
    new_free->next = pool->free_block;

    pool->free_block = new_free;
}

void pool_free(Pool *pool) {
    arena_free(&pool->arena);
    ZERO_STRUCT(pool);
}
