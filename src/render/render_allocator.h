#ifndef RENDER_ALLOCATOR_H
#define RENDER_ALLOCATOR_H

#include "core/common.h"
#include "render/render_common.h"

// forward declaration
typedef struct RND_Context RND_Context;

typedef struct RND_Arena RND_Arena;
struct RND_Arena {
    VkPhysicalDeviceMemoryProperties device_memory_props;
    VkDeviceMemory memory;
    u64 capacity;
};

// TODO(ss): actually implement management of memory on the allocator side instead of caller side
RND_Arena rnd_arena_init(RND_Context *rc, u64 initial_size);
void rnd_arena_free(RND_Context *rc, RND_Arena *allocator);

// Finds best memory type, etc, binds
void rnd_arena_alloc_image(RND_Arena *arena, RND_Context *rc, VkImageCreateInfo info,
                           VkMemoryPropertyFlags memory_prop_flag, VkImage *image,
                           VkDeviceMemory memory);

void rnd_arena_pop(RND_Arena *arena, RND_Context *rc, u64 size);
void rnd_arena_pop_to(RND_Arena *arena, RND_Context *rc, u64 size);

#endif // RENDER_ALLOCATOR_H
