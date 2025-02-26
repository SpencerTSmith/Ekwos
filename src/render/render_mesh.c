#include "render/render_mesh.h"

#include "core/log.h"
#include "render/render_context.h"

const VkVertexInputBindingDescription g_rnd_vertex_binding_descs[RND_VERTEX_BINDINGS_NUM] = {
    {
        .binding = 0,
        .stride = sizeof(RND_Vertex),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    },
};

const VkVertexInputAttributeDescription g_rnd_vertex_attrib_descs[RND_VERTEX_ATTRIBUTES_NUM] = {
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
    {
        .binding = 0,
        .location = 2,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(RND_Vertex, normal),
    },
    {
        .binding = 0,
        .location = 3,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(RND_Vertex, uv),
    },
};

void rnd_mesh_init(RND_Context *rc, RND_Mesh *mesh, RND_Vertex *verts, u32 vert_count, u32 *indices,
                   u32 index_count) {
  mesh->vertex_buffer = rnd_buffer_make_vertex(rc, verts, vert_count);

  // If we are using an index buffer
  if (indices != NULL && index_count > 0) {
    mesh->index_buffer = rnd_buffer_make_index(rc, indices, index_count);
  }
}

void rnd_mesh_bind(RND_Context *rc, RND_Mesh *mesh) {
  ASSERT(mesh->vertex_buffer.buffer != VK_NULL_HANDLE && mesh->vertex_buffer.item_count > 0,
         "Tried to bind vertex buffer with no allocated vertices");
  VkBuffer buffers[] = {mesh->vertex_buffer.buffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(rnd_get_current_cmd(rc), 0, 1, buffers, offsets);

  // TODO(ss): Probably not good to have this branch, just always used indexed meshes?
  if (mesh->index_buffer.buffer != VK_NULL_HANDLE && mesh->index_buffer.item_count > 0) {
    vkCmdBindIndexBuffer(rnd_get_current_cmd(rc), mesh->index_buffer.buffer, 0,
                         VK_INDEX_TYPE_UINT32);
  }
}

void rnd_mesh_draw(RND_Context *rc, RND_Mesh *mesh) {
  if (mesh->index_buffer.buffer != VK_NULL_HANDLE && mesh->index_buffer.item_count > 0) {
    vkCmdDrawIndexed(rnd_get_current_cmd(rc), mesh->index_buffer.item_count, 1, 0, 0, 0);
  } else {
    ASSERT(mesh->vertex_buffer.buffer != VK_NULL_HANDLE && mesh->vertex_buffer.item_count > 0,
           "Tried to draw vertex buffer with no allocated vertices");
    vkCmdDraw(rnd_get_current_cmd(rc), mesh->vertex_buffer.item_count, 1, 0, 0);
  }
}

void rnd_mesh_free(RND_Context *rc, RND_Mesh *mesh) {
  rnd_buffer_free(rc, &mesh->vertex_buffer);
  rnd_buffer_free(rc, &mesh->index_buffer);

  ZERO_STRUCT(mesh);
}

void rnd_mesh_default_cube(RND_Context *rc, RND_Mesh *mesh) {
  RND_Vertex verts[] = {
      {.position = vec3(1.f, -1.f, 1.f),
       .color = vec3(1.0f, .5f, .5f),
       .normal = vec3(0.577f, -0.577f, 0.577f)},
      {.position = vec3(1.f, 1.f, 1.f),
       .color = vec3(.1f, .1f, 0.8f),
       .normal = vec3(0.577f, 0.577f, 0.577f)},
      {.position = vec3(1.f, -1.f, -1.f),
       .color = vec3(.1f, .8f, .2f),
       .normal = vec3(0.577f, -0.577f, -0.577f)},
      {.position = vec3(1.f, 1.f, -1.f),
       .color = vec3(1.0f, 1.0f, .2f),
       .normal = vec3(0.577f, 0.577f, -0.577f)},
      {.position = vec3(-1.f, -1.f, 1.f),
       .color = vec3(0.f, 1.0f, 1.f),
       .normal = vec3(-0.577f, -0.577f, 0.577f)},
      {.position = vec3(-1.f, 1.f, 1.f),
       .color = vec3(1.0f, .5f, .2f),
       .normal = vec3(-0.577f, -0.577f, 0.577f)},
      {.position = vec3(-1.f, -1.f, -1.f),
       .color = vec3(1.f, .5f, 1.f),
       .normal = vec3(-0.577f, -0.577f, -0.577f)},
      {.position = vec3(-1.f, 1.f, -1.f),
       .color = vec3(1.f, 0.f, .2f),
       .normal = vec3(-0.577f, 0.577f, -0.577f)},
  };

  u32 indices[] = {
      4, 2, 0, 2, 7, 3, 6, 5, 7, 1, 7, 5, 0, 3, 1, 4, 1, 5,
      4, 6, 2, 2, 6, 7, 6, 4, 5, 1, 3, 7, 0, 2, 3, 4, 0, 1,
  };

  mesh->vertex_buffer = rnd_buffer_make_vertex(rc, verts, STATIC_ARRAY_COUNT(verts));
  mesh->index_buffer = rnd_buffer_make_index(rc, indices, STATIC_ARRAY_COUNT(indices));
}
