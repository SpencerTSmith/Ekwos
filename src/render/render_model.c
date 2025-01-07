#include "render/render_model.h"
#include "core/log.h"
#include <string.h>

const VkVertexInputBindingDescription vertex_binding_desc[VERTEX_BINDING_NUM] = {{
    .binding = 0,
    .stride = sizeof(Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
}};
const VkVertexInputAttributeDescription vertex_attrib_desc[VERTEX_ATTRIBUTES_NUM] = {
    {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(Vertex, position),
    },
    {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(Vertex, color),
    },
};

static void create_vertex_buffers(Render_Context *rc, Render_Model *model, Vertex *verts,
                                  u32 vert_count);

void render_model_init(Render_Context *rc, Render_Model *model, Vertex *verts, u32 vert_count) {
    create_vertex_buffers(rc, model, verts, vert_count);
}

void render_model_bind(Render_Context *rc, Render_Model *model) {
    VkBuffer buffers[] = {model->vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(rc->command_buffers[rc->swap.curr_frame], 0, 1, buffers, offsets);
}

void render_model_draw(Render_Context *rc, Render_Model *model) {
    vkCmdDraw(rc->command_buffers[rc->swap.curr_frame], model->vertex_count, 1, 0, 0);
}

void render_model_free(Render_Context *rc, Render_Model *model) {
    vkDestroyBuffer(rc->logical, model->vertex_buffer, NULL);
    vkFreeMemory(rc->logical, model->memory, NULL);
}

static void create_buffer(Render_Context *rc, VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkBuffer *buffer,
                          VkDeviceMemory *buffer_memory) {
    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult result = vkCreateBuffer(rc->logical, &buffer_info, NULL, buffer);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create buffer");
        return;
    }

    VkMemoryRequirements mem_reqs = {0};
    vkGetBufferMemoryRequirements(rc->logical, *buffer, &mem_reqs);

    u32 mem_type_idx = 0;
    VkPhysicalDeviceMemoryProperties mem_properties = {0};
    vkGetPhysicalDeviceMemoryProperties(rc->physical, &mem_properties);

    for (u32 i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((mem_reqs.memoryTypeBits & (1 << i)) &&
            (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            mem_type_idx = i;
        }
    }

    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_reqs.size;
    alloc_info.memoryTypeIndex = mem_type_idx;

    result = vkAllocateMemory(rc->logical, &alloc_info, NULL, buffer_memory);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate memory for buffer");
        return;
    }

    vkBindBufferMemory(rc->logical, *buffer, *buffer_memory, 0);
}

static void create_vertex_buffers(Render_Context *rc, Render_Model *model, Vertex *verts,
                                  u32 vert_count) {
    model->vertex_count = vert_count;
    if (vert_count < 3) {
        LOG_ERROR("Vertex count must be greater than or equal to 3");
        return;
    }

    VkDeviceSize buffer_size = sizeof(Vertex) * vert_count;
    create_buffer(rc, buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &model->vertex_buffer, &model->memory);

    // Map out memory to copy vertex info into
    void *data = NULL;
    vkMapMemory(rc->logical, model->memory, 0, buffer_size, 0, &data);
    memcpy(data, verts, buffer_size);
    vkUnmapMemory(rc->logical, model->memory);
}
