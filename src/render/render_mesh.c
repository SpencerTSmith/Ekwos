#include "render/render_mesh.h"

#include "core/log.h"
#include "render/render_context.h"

const VkVertexInputBindingDescription g_vertex_binding_desc[RND_VERTEX_BINDING_NUM] = {
    {
        .binding = 0,
        .stride = sizeof(RND_Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    },
};

const VkVertexInputAttributeDescription g_vertex_attrib_desc[RND_VERTEX_ATTRIBUTES_NUM] = {
    {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(RND_Vertex, position),
    },
    {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(RND_Vertex, color),
    },
};

static void create_vertex_buffers(RND_Context *rc, RND_Mesh *mesh, RND_Vertex *verts,
                                  u32 vert_count);

void rnd_mesh_init(RND_Context *rc, RND_Mesh *mesh, RND_Vertex *verts, u32 vert_count) {
    create_vertex_buffers(rc, mesh, verts, vert_count);
}

void rnd_mesh_bind(RND_Context *rc, RND_Mesh *mesh) {
    VkBuffer buffers[] = {mesh->vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(rnd_get_current_cmd(rc), 0, 1, buffers, offsets);
}

void rnd_mesh_draw(RND_Context *rc, RND_Mesh *mesh) {
    vkCmdDraw(rnd_get_current_cmd(rc), mesh->vertex_count, 1, 0, 0);
}

void rnd_mesh_free(RND_Context *rc, RND_Mesh *mesh) {
    vkDestroyBuffer(rc->logical, mesh->vertex_buffer, NULL);
    vkFreeMemory(rc->logical, mesh->memory, NULL);
    ZERO_STRUCT(mesh);
}

void rnd_mesh_cube(RND_Context *rc, RND_Mesh *mesh) {
    RND_Vertex verts[] = {

        // left face (white)
        {.position = {{-.5f, -.5f, -.5f}}, {{.9f, .9f, .9f}}},
        {.position = {{-.5f, .5f, .5f}}, {{.9f, .9f, .9f}}},
        {.position = {{-.5f, -.5f, .5f}}, {{.9f, .9f, .9f}}},
        {.position = {{-.5f, -.5f, -.5f}}, {{.9f, .9f, .9f}}},
        {.position = {{-.5f, .5f, -.5f}}, {{.9f, .9f, .9f}}},
        {.position = {{-.5f, .5f, .5f}}, {{.9f, .9f, .9f}}},

        // right face (yellow)
        {.position = {{.5f, -.5f, -.5f}}, {{.8f, .8f, .1f}}},
        {.position = {{.5f, .5f, .5f}}, {{.8f, .8f, .1f}}},
        {.position = {{.5f, -.5f, .5f}}, {{.8f, .8f, .1f}}},
        {.position = {{.5f, -.5f, -.5f}}, {{.8f, .8f, .1f}}},
        {.position = {{.5f, .5f, -.5f}}, {{.8f, .8f, .1f}}},
        {.position = {{.5f, .5f, .5f}}, {{.8f, .8f, .1f}}},

        // top face (orange, remember y axis points down)
        {.position = {{-.5f, -.5f, -.5f}}, {{.9f, .6f, .1f}}},
        {.position = {{.5f, -.5f, .5f}}, {{.9f, .6f, .1f}}},
        {.position = {{-.5f, -.5f, .5f}}, {{.9f, .6f, .1f}}},
        {.position = {{-.5f, -.5f, -.5f}}, {{.9f, .6f, .1f}}},
        {.position = {{.5f, -.5f, -.5f}}, {{.9f, .6f, .1f}}},
        {.position = {{.5f, -.5f, .5f}}, {{.9f, .6f, .1f}}},

        // bottom face (red)
        {.position = {{-.5f, .5f, -.5f}}, {{.8f, .1f, .1f}}},
        {.position = {{.5f, .5f, .5f}}, {{.8f, .1f, .1f}}},
        {.position = {{-.5f, .5f, .5f}}, {{.8f, .1f, .1f}}},
        {.position = {{-.5f, .5f, -.5f}}, {{.8f, .1f, .1f}}},
        {.position = {{.5f, .5f, -.5f}}, {{.8f, .1f, .1f}}},
        {.position = {{.5f, .5f, .5f}}, {{.8f, .1f, .1f}}},

        // nose face (blue)
        {.position = {{-.5f, -.5f, 0.5f}}, {{.1f, .1f, .8f}}},
        {.position = {{.5f, .5f, 0.5f}}, {{.1f, .1f, .8f}}},
        {.position = {{-.5f, .5f, 0.5f}}, {{.1f, .1f, .8f}}},
        {.position = {{-.5f, -.5f, 0.5f}}, {{.1f, .1f, .8f}}},
        {.position = {{.5f, -.5f, 0.5f}}, {{.1f, .1f, .8f}}},
        {.position = {{.5f, .5f, 0.5f}}, {{.1f, .1f, .8f}}},

        // tail face (green)
        {.position = {{-.5f, -.5f, -0.5f}}, {{.1f, .8f, .1f}}},
        {.position = {{.5f, .5f, -0.5f}}, {{.1f, .8f, .1f}}},
        {.position = {{-.5f, .5f, -0.5f}}, {{.1f, .8f, .1f}}},
        {.position = {{-.5f, -.5f, -0.5f}}, {{.1f, .8f, .1f}}},
        {.position = {{.5f, -.5f, -0.5f}}, {{.1f, .8f, .1f}}},
        {.position = {{.5f, .5f, -0.5f}}, {{.1f, .8f, .1f}}},
    };

    create_vertex_buffers(rc, mesh, verts, STATIC_ARRAY_COUNT(verts));
}

static void create_buffer(RND_Context *rc, VkDeviceSize size, VkBufferUsageFlags usage,
                          VkMemoryPropertyFlags properties, VkBuffer *buffer,
                          VkDeviceMemory *buffer_memory) {
    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK_ERROR(vkCreateBuffer(rc->logical, &buffer_info, NULL, buffer),
                   "Failed to create buffer");

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

    // TODO(ss): NOOOOOOO, fix this...
    VK_CHECK_ERROR(vkAllocateMemory(rc->logical, &alloc_info, NULL, buffer_memory),
                   "Failed to allocate memory for buffer");

    VK_CHECK_ERROR(vkBindBufferMemory(rc->logical, *buffer, *buffer_memory, 0),
                   "Failed to bind buffer memory");
}

static void create_vertex_buffers(RND_Context *rc, RND_Mesh *mesh, RND_Vertex *verts,
                                  u32 vert_count) {
    mesh->vertex_count = vert_count;
    if (vert_count < 3) {
        LOG_ERROR("Vertex count must be greater than or equal to 3");
        return;
    }

    VkDeviceSize buffer_size = sizeof(RND_Vertex) * vert_count;
    create_buffer(rc, buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                  &mesh->vertex_buffer, &mesh->memory);

    // Map out memory to copy vertex info into
    void *data = NULL;
    VK_CHECK_ERROR(vkMapMemory(rc->logical, mesh->memory, 0, buffer_size, 0, &data),
                   "Unable to map buffer memory for transfer");
    memcpy(data, verts, buffer_size);
    vkUnmapMemory(rc->logical, mesh->memory);
}
