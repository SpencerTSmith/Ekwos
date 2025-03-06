#ifndef ARENA_H
#define ARENA_H

#include "core/common.h"

#include <stdalign.h>

// TODO(ss): actually write the rest of this
typedef enum Arena_Flags {
  ARENA_FLAG_DEFAULTS = 0,
  ARENA_FLAG_FREE_LIST = (1 << 0),
  ARENA_FLAG_RESIZABLE = (1 << 1),
  ARENA_FLAG_CHAINABLE = (1 << 2),
} Arena_Flags;

typedef struct Arena Arena;
struct Arena {
  u8 *base_ptr;
  isize capacity;
  isize next_offset;
  Arena_Flags flags;
};

// TODO(ss): actually write this, allow giving a backing buffer
Arena arena_make(isize reserve_size, Arena_Flags flags);
void arena_free(Arena *arena);
void *arena_alloc(Arena *arena, isize size, isize alignment);
void arena_pop_to(Arena *arena, isize offset);
void arena_pop(Arena *arena, isize size);
void arena_clear(Arena *arena);

// Helper Macros ----------------------------------------------------------------

// specify the arena, the number of elements, and the type... c(ounted)alloc
#define arena_calloc(a, count, T) (T *)arena_alloc((a), sizeof(T) * (count), alignof(T))

// t(yped)alloc, useful for structs
#define arena_talloc(a, T) (arena_calloc(a, 1, T))

// Scratch Use Case -------------------------------------------------------------

// We just want some temporary memory
// ie we save the offset we wish to return to after using this arena as a scratch pad
typedef struct Scratch Scratch;
struct Scratch {
  Arena *arena;
  isize offset_save;
};

Scratch scratch_begin(Arena *arena);
void scratch_end(Scratch *scratch);

#endif // ARENA_H
