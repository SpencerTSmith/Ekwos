#ifndef RENDER_UPLOADER_H
#define RENDER_UPLOADER_H

#include "render/render_common.h"

typedef struct RND_Context RND_Context;

// This will get it's own command pool,
// in case we ever move this to it's own thread
typedef struct RND_Uploader RND_Uploader;
struct RND_Uploader {
  // NOTE(ss): I don't particularly like storing this here, but it will make the api a little bit
  // better... need to think of nicer way to do this
  VkDevice device;

  VkCommandPool command_pool;
  VkCommandBuffer command_buffer;

  VkBuffer staging_buffer;
  VkDeviceMemory staging_memory;

  VkQueue transfer_q;
  u32 transfer_index;
  VkSemaphore transfer_finished_sem;
};

RND_Uploader rnd_uploader_create(RND_Context *rc);
void rnd_uploader_free(RND_Context *rc, RND_Uploader *uploader);

// NOTE(ss): It may be useful in the future to internally batch these up
// Also, should we replace this with just a generic void * with a size? Or are separate functions
// ok?
void rnd_upload_buffer(RND_Uploader *uploader, void *data, u64 data_size, VkBuffer buffer);

#endif // RENDER_UPLOADER_H
