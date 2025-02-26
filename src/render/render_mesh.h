#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#include "core/common.h"
#include "core/linear_algebra.h"

#include "render/render_buffer.h"
#include "render/render_context.h"

typedef struct RND_Vertex RND_Vertex;
struct RND_Vertex {
  vec3 position;
  vec3 color;
  vec3 normal;
  vec2 uv;
};

// Remember alignment shit
typedef struct RND_Push_Constants RND_Push_Constants;
struct RND_Push_Constants {
  mat4 clip_transform;
  mat4 normal_matrix; // For some reason only works when its a mat4?!
};

typedef struct RND_Mesh RND_Mesh;
struct RND_Mesh {
  RND_Buffer vertex_buffer;
  RND_Buffer index_buffer;
};

enum RND_Mesh_Constants {
  RND_VERTEX_BINDINGS_NUM = 1,
  RND_VERTEX_ATTRIBUTES_NUM = 4,
};
extern const VkVertexInputBindingDescription g_rnd_vertex_binding_descs[RND_VERTEX_BINDINGS_NUM];
extern const VkVertexInputAttributeDescription g_rnd_vertex_attrib_descs[RND_VERTEX_ATTRIBUTES_NUM];

void rnd_mesh_init(RND_Context *rc, RND_Mesh *mesh, RND_Vertex *vertices, u32 vert_count,
                   u32 *indices, u32 index_count);
void rnd_mesh_bind(RND_Context *rc, RND_Mesh *mesh);
void rnd_mesh_draw(RND_Context *rc, RND_Mesh *mesh);
void rnd_mesh_free(RND_Context *rc, RND_Mesh *mesh);

void rnd_mesh_default_cube(RND_Context *rc, RND_Mesh *mesh);

#endif // RENDER_MODEL_H
