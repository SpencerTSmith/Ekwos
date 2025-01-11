#ifndef RENDER_ALLOCATOR_H
#define RENDER_ALLOCATOR_H

#include "core/common.h"
#include "render/render_common.h"

// forward declaration
typedef struct Render_Context Render_Context;

typedef struct Render_Arena Render_Arena;
struct Render_Arena {
    VkPhysicalDeviceMemoryProperties memory_props;
    VkDeviceMemory memory;
    u64 capacity;
};

// TODO(ss): actually implement management of memory on the allocator side instead of caller side
Render_Arena render_arena_init(Render_Context *rc, u64 initial_size);
void render_arena_free(Render_Context *rc, Render_Arena *allocator);

// Finds best memory type, etc, binds
void render_arena_alloc_image(Render_Arena *arena, Render_Context *rc, VkImageCreateInfo info,
                              VkMemoryPropertyFlags memory_prop_flag, VkImage *image,
                              VkDeviceMemory memory);

void render_arena_pop(Render_Arena *arena, Render_Context *rc, u64 size);
void render_arena_pop_to(Render_Arena *arena, Render_Context *rc, u64 size);

#endif // RENDER_ALLOCATOR_H
