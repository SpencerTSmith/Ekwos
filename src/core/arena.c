#include "arena.h"

#include "core/common.h"
#include "core/log.h"

#include <malloc.h>
#include <stdlib.h>

Arena arena_create(u64 reserve_size, Arena_Flags flags) {
    Arena arena = {0};

    // NOTE(spencer): this will return page-aligned memory (obviously) so I don't think it is
    // nessecary to make sure that the alignment suffices
    arena.base_ptr = calloc(reserve_size, 1);

    if (arena.base_ptr == NULL) {
        LOG_FATAL("Failed to allocate arena memory\n");
        exit(EXT_ARENA_ALLOCATION);
    }

    arena.capacity = reserve_size;
    arena.offset = 0;

    return arena;
}

void arena_free(Arena *arena) {
    free(arena->base_ptr);
    ZERO_STRUCT(arena);
}

void *arena_alloc(Arena *arena, u64 size, u64 alignment) {
    if (arena->base_ptr == NULL) {
        LOG_FATAL("Arena memory is null\n");
        exit(EXT_ARENA_ALLOCATION);
    }

    u64 aligned_offset = ALIGN_ROUND_UP(arena->offset, alignment);

    // Do we need a bigger buffer?
    if (aligned_offset + size > arena->capacity) {
        // TODO(spencer): Hmm, should we have arena chaining, fixed size arenas, offsets instead of
        // raw pointers?
        u64 needed_capacity = aligned_offset + size;

        LOG_DEBUG("Not enough memory in arena,\nNEED: %lu bytes\nHAVE: %lu bytes\n",
                  needed_capacity, arena->capacity);
        exit(EXT_ARENA_SIZE);
    }

    void *ptr = arena->base_ptr + aligned_offset;
    ZERO_SIZE(ptr, size); // make sure memory is zeroed out

    // now move the offset
    arena->offset = aligned_offset + size;

    return ptr;
}

void arena_pop_to(Arena *arena, u64 offset) {
    if (offset > arena->offset) {
        LOG_DEBUG("Failed to pop arena allocation, more than currently allocated\n");
        return;
    }

    arena->offset = offset;
}

void arena_pop(Arena *arena, u64 size) { arena_pop_to(arena, arena->offset - size); }

void arena_clear(Arena *arena) { arena->offset = 0; }

Scratch scratch_begin(Arena *arena) {
    Scratch scratch = {.arena = arena, .offset_save = arena->offset};
    return scratch;
}

void scratch_end(Scratch *scratch) {
    arena_pop_to(scratch->arena, scratch->offset_save);
    ZERO_STRUCT(scratch);
}
