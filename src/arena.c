#include "arena.h"

#include "common.h"

#include <malloc.h>
#include <stdlib.h>
#include <string.h>

// adds the alignment and then masks the lower bits to get the next (higher) multiple of that
// alignment... binary math
#define ALIGN_ROUND_UP(x, b) (((x) + (b) - 1) & (~((b) - 1)))

Arena arena_create(u64 reserve_size) {
    Arena arena = {0};

    // NOTE(spencer): this will return page-aligned memory (obviously) so I don't think it is
    // nessecary to make sure that the alignment suffices
    arena.base_ptr = calloc(reserve_size, 1);

    if (arena.base_ptr == NULL) {
        fprintf(stderr, "Failed to allocate arena memory\n");
        exit(EXT_ARENA_ALLOCATION);
    }

    arena.capacity = reserve_size;
    arena.offset = 0;

    return arena;
}

void arena_free(Arena *arena) {
    free(arena->base_ptr);
    memset(arena, 0, sizeof(Arena));
}

void *arena_alloc(Arena *arena, u64 size, u64 alignment) {
    if (arena->base_ptr == NULL) {
        fprintf(stderr, "Arena memory is null\n");
        exit(EXT_ARENA_ALLOCATION);
    }

    u64 aligned_offset = ALIGN_ROUND_UP(arena->offset, alignment);

    // Do we need a bigger buffer?
    if (aligned_offset + size > arena->capacity) {
        // TODO(spencer): Hmm, should we have arena chaining, fixed size arenas, offsets instead of
        // raw pointers?
        u64 needed_capacity = aligned_offset + size;

        // // kind of high for now, but once we can profile this... better number
        // u64 new_capacity = arena->capacity * 1.5;
        // new_capacity = new_capacity > needed_capacity ? new_capacity : needed_capacity;
        // arena->base_ptr = realloc(arena->base_ptr, new_capacity);

        fprintf(stderr, "Not enough memory in arena,\nNEED: %lu bytes\nHAVE: %lu bytes\n",
                needed_capacity, arena->capacity);
        exit(EXT_ARENA_SIZE);
    }

    void *ptr = arena->base_ptr + aligned_offset;
    // make sure memory is zeroed out
    memset(ptr, 0, size);

    // now move the offset
    arena->offset = aligned_offset + size;

    return ptr;
}

void arena_pop_to(Arena *arena, u64 offset) {
    if (offset > arena->offset) {
        fprintf(stderr, "Failed to pop arena allocation, more than currently allocated\n");
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
    memset(scratch, 0, sizeof(Scratch));
}
