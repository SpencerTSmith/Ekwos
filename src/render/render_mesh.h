#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#include "core/common.h"
#include "core/linear_algebra.h"

#include "render/render_common.h"
#include "render/render_context.h"

typedef struct RND_Vertex RND_Vertex;
struct RND_Vertex {
    vec3 position;
    vec3 color;
};

// Remember alignment shit
typedef struct RND_Push_Constants RND_Push_Constants;
struct RND_Push_Constants {
    mat4 transform;
    alignas(16) vec3 color;
};

typedef struct RND_Mesh RND_Mesh;
struct RND_Mesh {
    VkBuffer vertex_buffer;
    VkDeviceMemory memory;
    u32 vertex_count;
};

enum RND_Mesh_Constants {
    RND_VERTEX_BINDING_NUM = 1,
    RND_VERTEX_ATTRIBUTES_NUM = 2,
};
extern const VkVertexInputBindingDescription g_vertex_binding_desc[RND_VERTEX_BINDING_NUM];
extern const VkVertexInputAttributeDescription g_vertex_attrib_desc[RND_VERTEX_ATTRIBUTES_NUM];

// TODO(ss): Either need to use VMA (Vulkan Memory Allocator) library or create our own gpu memory
// allocator ASAP

void rnd_mesh_init(RND_Context *rc, RND_Mesh *mesh, RND_Vertex *verts, u32 vert_count);
void rnd_mesh_bind(RND_Context *rc, RND_Mesh *mesh);
void rnd_mesh_draw(RND_Context *rc, RND_Mesh *mesh);
void rnd_mesh_free(RND_Context *rc, RND_Mesh *mesh);

void rnd_mesh_cube(RND_Context *rc, RND_Mesh *mesh);

#endif // RENDER_MODEL_H
