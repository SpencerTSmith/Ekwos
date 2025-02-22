#include "core/thread_context.h"

thread_local Thread_Context *internal_tctx;

void thread_context_init(Thread_Context *tc) {
  // Does this work?
  function_local u32 thread_id = 0;

  tc->id = thread_id++;
  tc->scratch_arena = arena_create(GB(1), ARENA_FLAG_DEFAULTS);
  internal_tctx = tc;
}

void thread_context_free(void) {
  internal_tctx->id = UINT32_MAX; // Just in case we ever need to check if a thread is valid...
  arena_free(&internal_tctx->scratch_arena);
}

Scratch thread_get_scratch(void) { return scratch_begin(&internal_tctx->scratch_arena); }

void thread_end_scratch(Scratch *scratch) { scratch_end(scratch); }

Arena *thread_get_arena(void) { return &internal_tctx->scratch_arena; }
