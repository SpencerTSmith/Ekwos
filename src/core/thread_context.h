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

void thread_context_init(void);
void thread_context_free(void);

#endif // THREAD_CONTEXT_H
