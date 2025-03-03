#ifndef RENDER_BUFFER_H
#define RENDER_BUFFER_H

#include "render/render_common.h"

typedef struct RND_Context RND_Context;

// Going with the singular type, with enum... is this a good idea?
typedef enum RND_Buffer_Type {
  RND_BUFFER_VERTEX,
  RND_BUFFER_INDEX,
  RND_BUFFER_UNIFORM,
  RND_BUFFER_UNKOWN,
  RND_BUFFER_COUNT,
} RND_Buffer_Type;

typedef struct RND_Buffer RND_Buffer;
struct RND_Buffer {
  RND_Buffer_Type type;
  VkBuffer buffer;
  VkDeviceMemory memory;

  u32 item_count;
  VkDeviceSize item_size;
  VkDeviceSize aligned_size;
  VkDeviceSize buffer_size;

  // Only used if uniform really
  void *base_mapped;

  VkBufferUsageFlags usages;
  VkMemoryPropertyFlags memory_properties;
};

// Forward declaration for no recursive includes, sigh
typedef struct RND_Vertex RND_Vertex;

// TODO(ss): Is it a good idea to pass the allocator/uploader in as well? I wonder if in future
// There should be multiple allocators? The allocator interface so far kind of assumes there might
// be more by having to explicitly pass one in

// General purpose api (Mostly a nice wrapper around allocating and uploading data) -----------

// items argument can be NULL if you don't wish to write anything yet and only want to allocate
RND_Buffer rnd_buffer_make(RND_Context *rc, void *items, VkDeviceSize item_size, u32 item_count,
                           VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties,
                           VkDeviceSize min_alignment);
void rnd_buffer_free(RND_Context *rc, RND_Buffer *buffer);

// Helpful functions, less writing and some amount of added type safety
// These are device local (GPU)
RND_Buffer rnd_buffer_make_vertex(RND_Context *rc, RND_Vertex *vertices, u32 vert_count);
RND_Buffer rnd_buffer_make_index(RND_Context *rc, u32 *indices, u32 index_count);

// TODO(ss): This will not upload anything yet, only returning a mapped uniform buffer
RND_Buffer rnd_buffer_make_uniform(RND_Context *rc, VkDeviceSize per_frame_size, u32 frame_count);
void rnd_buffer_write_uniform(RND_Buffer *buffer);

#endif // RENDER_BUFFER_H
