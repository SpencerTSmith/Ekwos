#include "render/render_allocator.h"

#include "render/render_context.h"

RND_Arena rnd_arena_init(RND_Context *rc, u64 initial_size) {
    RND_Arena allocator = {0};
    vkGetPhysicalDeviceMemoryProperties(rc->physical, &allocator.device_memory_props);
    allocator.capacity = initial_size;

    // Initializing pools, arenas, other types of memory management here

    return allocator;
}

void rnd_arena_free(RND_Context *rc, RND_Arena *allocator) {
    // Destroy all linked device memories
}

void rnd_arena_alloc_image(RND_Arena *allocator, RND_Context *rc, VkImageCreateInfo info,
                           VkMemoryPropertyFlags memory_prop_flags, VkImage *image,
                           VkDeviceMemory memory) {
    assert(allocator->capacity != 0 && "Tried to use rendering memory arena before initialization");

    VK_CHECK_FATAL(vkCreateImage(rc->logical, &info, NULL, image), EXT_VK_IMAGE_CREATE,
                   "Failed to create image");

    VkMemoryRequirements memory_reqs = {0};
    vkGetImageMemoryRequirements(rc->logical, *image, &memory_reqs);

    u64 memory_type_index = UINT64_MAX;
    for (u32 i = 0; i < allocator->device_memory_props.memoryTypeCount; i++) {
        if ((memory_reqs.memoryTypeBits & (1 << i)) &&
            (allocator->device_memory_props.memoryTypes[i].propertyFlags & memory_prop_flags) ==
                memory_prop_flags) {
            memory_type_index = i;
            break;
        }
    }
    if (memory_type_index == UINT64_MAX) {
        LOG_FATAL("Failed to find suitable memory type index for image allocation");
    }

    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_reqs.size;
    alloc_info.memoryTypeIndex = memory_type_index;

    VK_CHECK_FATAL(vkAllocateMemory(rc->logical, &alloc_info, NULL, &memory), EXT_VK_ALLOCATION,
                   "Failed to allocate vulkan memory for image");

    VK_CHECK_FATAL(vkBindImageMemory(rc->logical, *image, memory, 0), EXT_VK_MEMORY_BIND,
                   "Failed to bind vulkan memory for image");
}
