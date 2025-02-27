#include "render/render_buffer.h"

#include "core/common.h"

#include "render/render_mesh.h"

RND_Buffer rnd_buffer_make(RND_Context *rc, void *items, VkDeviceSize item_size, u32 item_count,
                           VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties,
                           VkDeviceSize min_alignment) {
  RND_Buffer buf = {
      .item_size = item_size,
      .item_count = item_count,
      .usages = usage,
      .memory_properties = memory_properties,
      .alignment = min_alignment,
      .type = RND_BUFFER_UNKOWN,
  };

  VkDeviceSize buffer_size = item_size * item_count;

  VkBufferCreateInfo buffer_info = {0};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = buffer_size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  rnd_alloc_buffer(&rc->allocator, buffer_info, memory_properties, &buf.buffer, &buf.memory);
  rnd_upload_buffer(&rc->uploader, items, buffer_size, buf.buffer);

  return buf;
}

void rnd_buffer_free(RND_Context *rc, RND_Buffer *buffer) {
  if (buffer->item_count > 0 && buffer->buffer != VK_NULL_HANDLE &&
      buffer->memory != VK_NULL_HANDLE) {
    vkDestroyBuffer(rc->logical, buffer->buffer, NULL);
    vkFreeMemory(rc->logical, buffer->memory, NULL);
  } else {
    LOG_ERROR("Tried to free unallocated RND_Buffer");
  }

  ZERO_STRUCT(buffer);
}

RND_Buffer rnd_buffer_make_vertex(RND_Context *rc, RND_Vertex *vertices, u32 vert_count) {
  RND_Buffer vert_buf =
      rnd_buffer_make(rc, vertices, sizeof(vertices[0]), vert_count,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, alignof(vertices[0]));
  vert_buf.type = RND_BUFFER_VERTEX;

  return vert_buf;
}

RND_Buffer rnd_buffer_make_index(RND_Context *rc, u32 *indices, u32 index_count) {
  RND_Buffer idx_buf =
      rnd_buffer_make(rc, indices, sizeof(indices[0]), index_count,
                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, alignof(indices[0]));
  idx_buf.type = RND_BUFFER_INDEX;

  return idx_buf;
}

RND_Buffer rnd_buffer_make_uniform(void);
void rnd_buffer_write_uniform(RND_Buffer *buffer) {
  if (buffer->type != RND_BUFFER_UNIFORM) {
    LOG_ERROR("Tried to write uniform to non-uniform buffer");
    return;
  }
}
