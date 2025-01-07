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

typedef struct Render_Model Render_Model;
struct Render_Model {
    VkBuffer vertex_buffer;
    VkDeviceMemory memory;
    u32 vertex_count;
};

#define VERTEX_BINDING_NUM 1
#define VERTEX_ATTRIBUTES_NUM 2
extern const VkVertexInputBindingDescription vertex_binding_desc[VERTEX_BINDING_NUM];
extern const VkVertexInputAttributeDescription vertex_attrib_desc[VERTEX_ATTRIBUTES_NUM];

// TODO(ss): Either need to use VMA (Vulkan Memory Allocator Library) or create our own gpu memory
// allocator ASAP

void render_model_init(Render_Context *rc, Render_Model *model, Vertex *verts, u32 vert_count);
void render_model_bind(Render_Context *rc, Render_Model *model);
void render_model_draw(Render_Context *rc, Render_Model *model);
void render_model_free(Render_Context *rc, Render_Model *model);

#endif // RENDER_MODEL_H
