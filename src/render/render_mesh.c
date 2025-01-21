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

static void create_vertex_buffer(RND_Context *rc, RND_Mesh *mesh, RND_Vertex *verts,
                                 u32 vert_count);
static void create_index_buffer(RND_Context *rc, RND_Mesh *mesh, u32 *indexs, u32 index_count);

void rnd_mesh_init(RND_Context *rc, RND_Mesh *mesh, RND_Vertex *verts, u32 vert_count, u32 *indexs,
                   u32 indx_count) {
    create_vertex_buffer(rc, mesh, verts, vert_count);
    if (indexs != NULL && indx_count > 0) {
        create_index_buffer(rc, mesh, indexs, indx_count);
    }
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
    vkFreeMemory(rc->logical, mesh->vertex_memory, NULL);
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

        // bottom face (orange)
        {.position = {{-.5f, -.5f, -.5f}}, {{1.0f, .35f, .0f}}},
        {.position = {{.5f, -.5f, .5f}}, {{1.0f, .35f, .0f}}},
        {.position = {{-.5f, -.5f, .5f}}, {{1.0f, .35f, .0f}}},
        {.position = {{-.5f, -.5f, -.5f}}, {{1.0f, .35f, .0f}}},
        {.position = {{.5f, -.5f, -.5f}}, {{1.0f, .35f, .0f}}},
        {.position = {{.5f, -.5f, .5f}}, {{1.0f, .35f, .0f}}},

        // top face (red)
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

    create_vertex_buffer(rc, mesh, verts, STATIC_ARRAY_COUNT(verts));
}

static void create_vertex_buffer(RND_Context *rc, RND_Mesh *mesh, RND_Vertex *verts,
                                 u32 vert_count) {
    mesh->vertex_count = vert_count;
    if (vert_count < 3) {
        LOG_ERROR("Vertex count must be greater than or equal to 3");
        return;
    }

    VkDeviceSize buffer_size = sizeof(RND_Vertex) * vert_count;
    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    rnd_alloc_buffer(&rc->allocator, rc, buffer_info,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &mesh->vertex_buffer, &mesh->vertex_memory);

    // Map out memory to copy vertex info into
    void *data = NULL;
    VK_CHECK_ERROR(vkMapMemory(rc->logical, mesh->vertex_memory, 0, buffer_size, 0, &data),
                   "Unable to map buffer memory for transfer");
    memcpy(data, verts, buffer_size);
    vkUnmapMemory(rc->logical, mesh->vertex_memory);
}

static void create_index_buffer(RND_Context *rc, RND_Mesh *mesh, u32 *indexs, u32 index_count) {
    mesh->index_count = index_count;

    VkDeviceSize buffer_size = sizeof(u32) * index_count;
    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = buffer_size;
    buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    rnd_alloc_buffer(&rc->allocator, rc, buffer_info,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     &mesh->index_buffer, &mesh->index_memory);
}
