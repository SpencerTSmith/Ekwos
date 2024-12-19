#ifndef ARENA_H
#define ARENA_H

#include "common.h"

typedef struct Arena Arena;
struct Arena {
    void *base_ptr;
    u64 capacity;
    u64 offset;
};

void arena_create(Arena *arena, u64 reserve_size);
void arena_free(Arena *arena);

void *arena_alloc(Arena *arena, u64 size, u64 alignment);
void arena_pop(Arena *arena, u64 size);
void arena_clear(Arena *arena);

#endif // ARENA_H
