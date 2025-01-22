#include "render/render_uploader.h"

#include "render/render_context.h"

// Assumes corresponding queue has been "gotten"
RND_Uploader create_uploader(RND_Context *rc) {
    // This command pool is dependent on transfer operations
    VkCommandPoolCreateInfo pi = {0};
    pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pi.queueFamilyIndex = rc->uploader.transfer_index;

    VK_CHECK_FATAL(vkCreateCommandPool(rc->logical, &pi, NULL, &rc->uploader.command_pool),
                   EXT_VK_COMMAND_POOL, "Failed to create uploader command pool");
    LOG_DEBUG("Created uploader command pool");

    VkCommandBufferAllocateInfo ai = {0};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandBufferCount = 1;
    ai.commandPool = rc->uploader.command_pool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK_FATAL(vkAllocateCommandBuffers(rc->logical, &ai, &rc->uploader.command_buffer),
                   EXT_VK_COMMAND_BUFFER, "Failed to allocate uploader command buffer");
    LOG_DEBUG("Created uploader command buffer");

    VkSemaphoreCreateInfo si = {0};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_CHECK_FATAL(vkCreateSemaphore(rc->logical, &si, NULL, &rc->uploader.transfer_sem),
                   EXT_VK_SYNC_OBJECT, "Failed to create uploader transfer semaphore");
    LOG_DEBUG("Created uploader transfer semaphore");

    VkBufferCreateInfo bi = {0};
    bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bi.queueFamilyIndexCount = 1;
    bi.pQueueFamilyIndices = &rc->uploader.transfer_index;
    bi.size = RENDER_CONTEXT_STAGING_SIZE;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    rnd_alloc_buffer(&rc->allocator, rc, bi,
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                     &rc->uploader.staging_buffer, &rc->uploader.staging_memory);
}

static void destroy_uploader(RND_Context *rc) {
    if (rc->uploader.transfer_sem != VK_NULL_HANDLE) {
        vkDestroySemaphore(rc->logical, rc->uploader.transfer_sem, NULL);
    }
    if (rc->uploader.command_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(rc->logical, rc->uploader.command_pool, NULL);
    }
    if (rc->uploader.staging_memory != VK_NULL_HANDLE &&
        rc->uploader.staging_buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(rc->logical, rc->uploader.staging_buffer, NULL);
        vkFreeMemory(rc->logical, rc->uploader.staging_memory, NULL);
    }
}
