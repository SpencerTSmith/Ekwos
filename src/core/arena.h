#ifndef ARENA_H
#define ARENA_H

#include "common.h"

typedef struct Arena Arena;
struct Arena {
    u8 *base_ptr;
    u64 capacity;
    u64 offset;
};

Arena arena_create(u64 reserve_size);
void arena_free(Arena *arena);
void *arena_alloc(Arena *arena, u64 size, u64 align);
void arena_pop_to(Arena *arena, u64 offset);
void arena_pop(Arena *arena, u64 size);
void arena_clear(Arena *arena);

// Helper Macros //
#define arena_calloc(a, c, t) (t *)(arena_alloc((a), sizeof(t) * (c), alignof(t)))

// We just want some temporary, quickly deallocated memory
// ie we save the offset we wish to return to after using this arena as a scratch pad
typedef struct Scratch Scratch;
struct Scratch {
    Arena *arena;
    u64 offset_save;
};

Scratch scratch_begin(Arena *arena);
void scratch_end(Scratch *scratch);

#endif // ARENA_H
