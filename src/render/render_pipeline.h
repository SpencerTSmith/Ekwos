#ifndef PIPELINE_H
#define PIPELINE_H

#include "core/common.h"
#include "render/render_context.h"
#include "render/render_mesh.h"

typedef struct Render_Pipeline Render_Pipeline;
struct Render_Pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;
};

enum {
    MAX_DYNAMIC_PIPELINE_STATES = 2,
};

typedef struct Pipeline_Config Pipeline_Config;
struct Pipeline_Config {
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
    VkPipelineRasterizationStateCreateInfo rasterization_info;
    VkPipelineMultisampleStateCreateInfo multisample_info;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
    VkPipelineViewportStateCreateInfo viewport_info;
    VkPipelineColorBlendAttachmentState color_blend_attachment_state;
    VkDynamicState dynamic_states[MAX_DYNAMIC_PIPELINE_STATES];
    u32 dynamic_state_count;
};

typedef struct Shader_Code Shader_Code;
struct Shader_Code {
    u8 *data;
    u64 size;
};

// Will use a default configuration if NULL passed in for config parameter
Render_Pipeline render_pipeline_create(Arena *arena, Render_Context *rc,
                                       const char *vert_shader_path, const char *frag_shader_path,
                                       const Pipeline_Config *config);
void render_pipeline_free(Render_Context *render_context, Render_Pipeline *pipeline);

void render_pipeline_bind(Render_Context *render_context, Render_Pipeline *pipeline);

void render_push_constants(Render_Context *rc, Render_Pipeline *pl, Push_Constants push);

Shader_Code read_shader_file(Arena *arena, const char *file_path);
VkShaderModule create_shader_module(Shader_Code code, VkDevice device);

#endif // PIPELINE_H
