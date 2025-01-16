#ifndef THREAD_CONTEXT_H
#define THREAD_CONTEXT_H

#include "core/arena.h"
#include "core/common.h"

// TODO(ss): actually implement this
typedef struct Thread_Context Thread_Context;
struct Thread_Context {
    u32 id;
    Arena scratch_arena;
};

void thread_context_init(Thread_Context *thread_context);
void thread_context_free(void);

// NOTE(ss): A linear allocator for any dynamic scratch work you might want to do...
// Use if a function does not need any allocations that stick around, and remember
// to call end_scratch after
Scratch thread_get_scratch(void);
void thread_end_scratch(Scratch *scratch);

// Really shouldn't need this I don't think, but just in case
Arena *thread_get_arena(void);
#endif // THREAD_CONTEXT_H
