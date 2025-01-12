#ifndef ARENA_H
#define ARENA_H

#include "core/common.h"
#include <stdalign.h>

// TODO(ss): actually write the rest of this
typedef u64 Arena_Flags;
enum Arena_Flags {
    ARENA_FLAG_DEFAULTS = 0,
    ARENA_FLAG_FREE_LIST = (1 << 0),
    ARENA_FLAG_RESIZABLE = (1 << 1),
    ARENA_FLAG_CHAINABLE = (1 << 2),
};

typedef struct Arena Arena;
struct Arena {
    u8 *base_ptr;
    u64 capacity;
    u64 offset;
};

Arena arena_create(u64 reserve_size, Arena_Flags flags);
void arena_free(Arena *arena);
void *arena_alloc(Arena *arena, u64 size, u64 align);
void arena_pop_to(Arena *arena, u64 offset);
void arena_pop(Arena *arena, u64 size);
void arena_clear(Arena *arena);

// Helper Macros //

// specify the arena, the number of elements, and the type... c(ounted)alloc
#define arena_calloc(a, count, T) (T *)arena_alloc((a), sizeof(T) * (count), alignof(T))

// We just want some temporary memory
// ie we save the offset we wish to return to after using this arena as a scratch pad
typedef struct Scratch Scratch;
struct Scratch {
    Arena *arena;
    u64 offset_save;
};

Scratch scratch_begin(Arena *arena);
void scratch_end(Scratch *scratch);

#endif // ARENA_H
