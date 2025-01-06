#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#include "core/common.h"

typedef struct Render_Model Render_Model;
struct Render_Model {
    VkBuffer vertex_buffer;
    VkDeviceMemory memory;
    u32 vertex_count;
};

void model_bind(Render_Context *rc, Render_Model *model);
void model_draw(Render_Context *rc, Render_Model *model);

#endif // RENDER_MODEL_H
