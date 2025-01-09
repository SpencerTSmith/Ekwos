#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#include "core/common.h"
#include "core/linear_algebra.h"
#include "render/render_context.h"

typedef struct Vertex Vertex;
struct Vertex {
    vec2 position;
    vec3 color;
};

// Remember alignment shit
typedef struct Push_Constants Push_Constants;
struct Push_Constants {
    vec2 offset;
    alignas(16) vec3 color;
};

typedef struct Render_Mesh Render_Mesh;
struct Render_Mesh {
    VkBuffer vertex_buffer;
    VkDeviceMemory memory;
    u32 vertex_count;
};

#define VERTEX_BINDING_NUM 1
#define VERTEX_ATTRIBUTES_NUM 2
extern const VkVertexInputBindingDescription g_vertex_binding_desc[VERTEX_BINDING_NUM];
extern const VkVertexInputAttributeDescription g_vertex_attrib_desc[VERTEX_ATTRIBUTES_NUM];

// TODO(ss): Either need to use VMA (Vulkan Memory Allocator Library) or create our own gpu memory
// allocator ASAP

void render_mesh_init(Render_Context *rc, Render_Mesh *mesh, Vertex *verts, u32 vert_count);
void render_mesh_bind(Render_Context *rc, Render_Mesh *mesh);
void render_mesh_draw(Render_Context *rc, Render_Mesh *mesh);
void render_mesh_free(Render_Context *rc, Render_Mesh *mesh);

#endif // RENDER_MODEL_H
