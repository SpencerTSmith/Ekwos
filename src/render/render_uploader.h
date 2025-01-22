#ifndef RENDER_UPLOADER_H
#define RENDER_UPLOADER_H

#include "render/render_common.h"

typedef struct RND_Context RND_Context;
typedef struct RND_Vertex RND_Vertex;

// This will get it's own command pool,
// in case we ever move this to it's own thread
typedef struct RND_Uploader RND_Uploader;
struct RND_Uploader {
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkBuffer staging_buffer;
    VkDeviceMemory staging_memory;
    VkQueue transfer_q;
    u32 transfer_index;
    VkSemaphore transfer_sem;
};

RND_Uploader rnd_uploader_create(RND_Context *rc);
void rnd_uploader_free(RND_Uploader *uploader);

// NOTE(ss): It may be useful in the future to internally batch these up
void rnd_upload_vertex_buffer(RND_Uploader *uploader, RND_Vertex *verts, u32 vert_count,
                              VkBuffer vertex_buffer);
void rnd_upload_index_buffer(RND_Uploader *uploader, u32 *indexs, u32 index_count,
                             VkBuffer index_buffer);

#endif // RENDER_UPLOADER_H
