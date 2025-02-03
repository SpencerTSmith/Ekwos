#include "render/render_uploader.h"

#include "render/render_context.h"

// Assumes corresponding queue has been "gotten"
RND_Uploader rnd_uploader_create(RND_Context *rc) {
    RND_Uploader uploader = {0};

    vkGetDeviceQueue(rc->logical, rc->uploader.transfer_index, 0, &uploader.transfer_q);
    LOG_DEBUG("Got transfer device queue with family index %u", rc->present_index);

    uploader.device = rc->logical;
    // This command pool is dependent on transfer operations
    VkCommandPoolCreateInfo pi = {0};
    pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pi.queueFamilyIndex = uploader.transfer_index;

    VK_CHECK_FATAL(vkCreateCommandPool(rc->logical, &pi, NULL, &uploader.command_pool),
                   EXT_VK_COMMAND_POOL, "Failed to create uploader command pool");
    LOG_DEBUG("Created uploader command pool");

    VkCommandBufferAllocateInfo ai = {0};
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandBufferCount = 1;
    ai.commandPool = uploader.command_pool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK_FATAL(vkAllocateCommandBuffers(rc->logical, &ai, &uploader.command_buffer),
                   EXT_VK_COMMAND_BUFFER, "Failed to allocate uploader command buffer");
    LOG_DEBUG("Created uploader command buffer");

    VkSemaphoreCreateInfo si = {0};
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_CHECK_FATAL(vkCreateSemaphore(rc->logical, &si, NULL, &uploader.transfer_finished_sem),
                   EXT_VK_SYNC_OBJECT, "Failed to create uploader transfer semaphore");
    LOG_DEBUG("Created uploader transfer semaphore");

    VkBufferCreateInfo bi = {0};
    bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bi.queueFamilyIndexCount = 1;
    bi.pQueueFamilyIndices = &uploader.transfer_index;
    bi.size = RENDER_CONTEXT_STAGING_SIZE;
    bi.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    bi.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // Uploading here, so it is a source

    rnd_alloc_buffer(&rc->allocator, bi,
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
                     &uploader.staging_buffer, &uploader.staging_memory);

    return uploader;
}

void rnd_uploader_free(RND_Context *rc, RND_Uploader *uploader) {
    if (uploader->transfer_finished_sem != VK_NULL_HANDLE) {
        vkDestroySemaphore(rc->logical, uploader->transfer_finished_sem, NULL);
    }
    if (uploader->command_pool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(rc->logical, uploader->command_pool, NULL);
    }
    if (uploader->staging_memory != VK_NULL_HANDLE && uploader->staging_buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(rc->logical, uploader->staging_buffer, NULL);
        vkFreeMemory(rc->logical, uploader->staging_memory, NULL);
    }
}

void rnd_upload_buffer(RND_Uploader *uploader, void *data, u64 data_size, VkBuffer buffer) {

    // Map our data into the staging buffer
    void *mapped = NULL;
    VK_CHECK_ERROR(
        vkMapMemory(uploader->device, uploader->staging_memory, 0, data_size, 0, &mapped),
        "Unable to map memory for GPU upload");
    memcpy(mapped, data, data_size);
    vkUnmapMemory(uploader->device, uploader->staging_memory);

    // NOTE(ss): May want to move the beginning of the command out of
    // here, batch it up
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK_ERROR(vkBeginCommandBuffer(uploader->command_buffer, &begin_info),
                   "Failed to begin command buffer recording for GPU upload");

    // Again in future if we want to batch uploads, this is where we can do that
    VkBufferCopy copy_region = {0};
    copy_region.size = data_size;
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    vkCmdCopyBuffer(uploader->command_buffer, uploader->staging_buffer, buffer, 1, &copy_region);

    VK_CHECK_ERROR(vkEndCommandBuffer(uploader->command_buffer),
                   "Failed to end command buffer recording for GPU upload");

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &uploader->command_buffer;

    VK_CHECK_ERROR(vkQueueSubmit(uploader->transfer_q, 1, &submit_info, VK_NULL_HANDLE),
                   "Failed to submit GPU upload on transfer queue");

    // TODO(ss): Figure out more competent syncronization, a fence maybe, but I'm thinking this
    // should be possible with a sempahore
    VK_CHECK_ERROR(vkQueueWaitIdle(uploader->transfer_q), "Failed to wait for transfer queue idle");
}
