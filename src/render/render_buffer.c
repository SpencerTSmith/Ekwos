#include "render/render_buffer.h"

#include "core/common.h"

#include "render/render_mesh.h"

RND_Buffer rnd_buffer_make(RND_Context *rc, void *items, RND_size item_size, u32 item_count,
                           VkBufferUsageFlags usage, VkMemoryPropertyFlags memory_properties,
                           RND_size min_alignment) {
  RND_Buffer buf = {
      .item_size = item_size,
      .item_count = item_count,
      .usages = usage,
      .memory_properties = memory_properties,
      .type = RND_BUFFER_UNKOWN,
  };
  buf.aligned_item_size = ALIGN_ROUND_UP(buf.item_size, min_alignment);

  buf.buffer_size = buf.aligned_item_size * buf.item_count;

  VkBufferCreateInfo buffer_info = {0};
  buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_info.size = buf.buffer_size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  rnd_alloc_buffer(&rc->allocator, buffer_info, memory_properties, &buf.buffer, &buf.memory);
  if (memory_properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT &&
      usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT && items != NULL)
    rnd_upload_buffer(&rc->uploader, items, buf.buffer_size, buf.buffer);

  LOG_DEBUG("Allocated and uploaded buffer with size: %lu, item count: %u, item size: %lu, aligned "
            "size: %lu",
            buf.buffer_size, buf.item_count, buf.item_size, buf.aligned_item_size);

  return buf;
}

void rnd_buffer_free(RND_Context *rc, RND_Buffer *buffer) {
  if (buffer->item_count > 0 && buffer->buffer != VK_NULL_HANDLE &&
      buffer->memory != VK_NULL_HANDLE) {

    if (buffer->type == RND_BUFFER_UNIFORM || buffer->type == RND_BUFFER_STAGING)
      vkUnmapMemory(rc->logical, buffer->memory);

    vkDestroyBuffer(rc->logical, buffer->buffer, NULL);
    vkFreeMemory(rc->logical, buffer->memory, NULL);
  } else {
    LOG_ERROR("Tried to free unallocated RND_Buffer");
  }

  ZERO_STRUCT(buffer);
}

RND_Buffer rnd_buffer_make_vertex(RND_Context *rc, RND_Vertex *vertices, u32 vert_count) {
  // No alignment requirement
  RND_Buffer vert_buf =
      rnd_buffer_make(rc, vertices, sizeof(vertices[0]), vert_count,
                      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1);
  vert_buf.type = RND_BUFFER_VERTEX;
  LOG_DEBUG("Above buffer was vertex");

  return vert_buf;
}

RND_Buffer rnd_buffer_make_index(RND_Context *rc, u32 *indices, u32 index_count) {
  // No alignment requirement
  RND_Buffer idx_buf =
      rnd_buffer_make(rc, indices, sizeof(indices[0]), index_count,
                      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 1);
  idx_buf.type = RND_BUFFER_INDEX;
  LOG_DEBUG("Above buffer was indices");

  return idx_buf;
}

RND_Buffer rnd_buffer_make_GUBO(RND_Context *rc, RND_size per_frame_size, u32 frame_count) {
  VkPhysicalDeviceProperties props = {0};
  vkGetPhysicalDeviceProperties(rc->physical, &props);

  RND_Buffer uni_buf =
      rnd_buffer_make(rc, NULL, per_frame_size, frame_count, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      props.limits.minUniformBufferOffsetAlignment);
  uni_buf.type = RND_BUFFER_UNIFORM;
  VK_CHECK_ERROR(
      vkMapMemory(rc->logical, uni_buf.memory, 0, uni_buf.buffer_size, 0, &uni_buf.base_mapped),
      "Unable to map memory for Uniform Buffer");
  LOG_DEBUG("Above buffer was uniform");

  return uni_buf;
}

void rnd_buffer_write_offset(RND_Buffer *buffer, void *data, RND_size size, RND_size offset) {
  ASSERT(buffer->base_mapped != NULL, "Tried to write to non-memory-mapped RND_Buffer");

  RND_size true_offset = (RND_size)buffer->base_mapped + offset;
  memcpy((void *)true_offset, data, size);
}

void rnd_buffer_write_item_index(RND_Buffer *buffer, void *item, RND_size index) {
  ASSERT(buffer->aligned_item_size != 0,
         "Tried to write to RND_Buffer index when aligned size is 0");

  RND_size offset = index * buffer->aligned_item_size;
  rnd_buffer_write_offset(buffer, item, buffer->item_size, offset);
}
