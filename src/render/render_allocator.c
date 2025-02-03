#include "render/render_allocator.h"

#include "render/render_context.h"

RND_Allocator rnd_allocator_create(RND_Context *rc, u64 initial_size) {
    RND_Allocator allocator = {0};
    vkGetPhysicalDeviceMemoryProperties(rc->physical, &allocator.device_memory_props);
    allocator.capacity = initial_size;
    allocator.device = rc->logical;

    // Initializing pools, arenas, other types of memory management here

    return allocator;
}

void rnd_allocator_free(RND_Allocator *allocator) {
    // Destroy all linked device memories
}

static u64 choose_memory_type(RND_Allocator *allocator, VkMemoryRequirements memory_reqs,
                              VkMemoryPropertyFlags memory_properties) {
    u64 memory_type_index = UINT64_MAX;
    for (u32 i = 0; i < allocator->device_memory_props.memoryTypeCount; i++) {
        if ((memory_reqs.memoryTypeBits & (1 << i)) &&
            (allocator->device_memory_props.memoryTypes[i].propertyFlags & memory_properties) ==
                memory_properties) {
            memory_type_index = i;
            break;
        }
    }

    if (memory_type_index == UINT64_MAX) {
        LOG_FATAL("Failed to find suitable memory type index for image allocation");
    }

    return memory_type_index;
}

void rnd_alloc_image(RND_Allocator *allocator, VkImageCreateInfo info,
                     VkMemoryPropertyFlags memory_properties, VkImage *image,
                     VkDeviceMemory *memory) {
    assert(allocator->capacity != 0 && "Tried to use rendering memory arena before initialization");

    VK_CHECK_FATAL(vkCreateImage(allocator->device, &info, NULL, image), EXT_VK_IMAGE_CREATE,
                   "Failed to create image");

    VkMemoryRequirements memory_reqs = {0};
    vkGetImageMemoryRequirements(allocator->device, *image, &memory_reqs);

    u64 memory_type_index = choose_memory_type(allocator, memory_reqs, memory_properties);

    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_reqs.size;
    alloc_info.memoryTypeIndex = memory_type_index;

    VK_CHECK_FATAL(vkAllocateMemory(allocator->device, &alloc_info, NULL, memory),
                   EXT_VK_ALLOCATION, "Failed to allocate vulkan memory for image");

    VK_CHECK_FATAL(vkBindImageMemory(allocator->device, *image, *memory, 0), EXT_VK_MEMORY_BIND,
                   "Failed to bind vulkan memory for image");
}

void rnd_alloc_buffer(RND_Allocator *allocator, VkBufferCreateInfo info,
                      VkMemoryPropertyFlags memory_properties, VkBuffer *buffer,
                      VkDeviceMemory *memory) {
    assert(allocator->capacity != 0 && "Tried to use rendering memory arena before initialization");

    VK_CHECK_FATAL(vkCreateBuffer(allocator->device, &info, NULL, buffer), EXT_VK_BUFFER_CREATE,
                   "Failed to create buffer");

    VkMemoryRequirements mem_reqs = {0};
    vkGetBufferMemoryRequirements(allocator->device, *buffer, &mem_reqs);

    u32 mem_type_idx = choose_memory_type(allocator, mem_reqs, memory_properties);

    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = mem_type_idx;

    VK_CHECK_FATAL(vkAllocateMemory(allocator->device, &alloc_info, NULL, memory),
                   EXT_VK_ALLOCATION, "Failed to allocate memory for buffer");

    VK_CHECK_FATAL(vkBindBufferMemory(allocator->device, *buffer, *memory, 0), EXT_VK_MEMORY_BIND,
                   "Failed to bind buffer memory");
}
