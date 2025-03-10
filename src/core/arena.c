#include "arena.h"

#include "core/common.h"
#include "core/log.h"

#include <malloc.h>
#include <stdlib.h>

Arena arena_make(isize reserve_size, Arena_Flags flags) {
  Arena arena = {0};

  // NOTE(ss): this will return page-aligned memory (obviously) so I don't think it is
  // nessecary to make sure that the alignment suffices
  arena.base = calloc(reserve_size, 1);

  if (arena.base == NULL) {
    LOG_FATAL("Failed to allocate arena memory", EXT_ARENA_ALLOCATION);
  }

  arena.capacity = reserve_size;
  arena.next_offset = 0;
  arena.flags = flags;

  return arena;
}

void arena_free(Arena *arena) {
  if (!(arena->flags & ARENA_FLAG_BACKING))
    free(arena->base);

  ZERO_STRUCT(arena);
}

Arena arena_make_backing(void *backing, isize size) {
  Arena arena = {0};

  arena.base = (u8 *)backing;
  arena.capacity = size;
  arena.next_offset = 0;
  arena.flags = ARENA_FLAG_BACKING;

  return arena;
}

void *arena_alloc(Arena *arena, isize size, isize alignment) {
  ASSERT(arena->base != NULL, "Arena memory is null");

  isize aligned_offset = ALIGN_ROUND_UP(arena->next_offset, alignment);

  // Do we need a bigger buffer?
  if (aligned_offset + size > arena->capacity) {
    // TODO(ss): Hmm, should we have arena chaining, fixed size arenas, offsets instead of
    // raw pointers?
    u64 needed_capacity = aligned_offset + size;

    LOG_FATAL("Not enough memory in arena,\nNEED: %l bytes\nHAVE: %l bytes", needed_capacity,
              arena->capacity);
    exit(EXT_ARENA_SIZE);
  }

  void *ptr = arena->base + aligned_offset;
  ZERO_SIZE(ptr, size); // make sure memory is zeroed out

  // now move the offset
  arena->next_offset = aligned_offset + size;

  return ptr;
}

void arena_pop_to(Arena *arena, isize offset) {
  ASSERT(offset < arena->next_offset,
         "Failed to pop arena allocation, more than currently allocated");

  // Should we zero out the memory?
  arena->next_offset = offset;
}

void arena_pop(Arena *arena, isize size) { arena_pop_to(arena, arena->next_offset - size); }

void arena_clear(Arena *arena) { arena->next_offset = 0; }

Scratch scratch_begin(Arena *arena) {
  Scratch scratch = {.arena = arena, .offset_save = arena->next_offset};
  return scratch;
}

void scratch_end(Scratch *scratch) {
  arena_pop_to(scratch->arena, scratch->offset_save);
  ZERO_STRUCT(scratch);
}
