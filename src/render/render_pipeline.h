#ifndef PIPELINE_H
#define PIPELINE_H

#include "core/common.h"
#include "render_context.h"

typedef struct Render_Pipeline Render_Pipeline;
struct Render_Pipeline {
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
};

typedef struct Pipeline_Config Pipeline_Config;
struct Pipeline_Config {
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
    VkPipelineRasterizationStateCreateInfo rasterization_info;
    VkPipelineMultisampleStateCreateInfo multisample_info;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
    VkPipelineLayoutCreateInfo pipeline_layout_info;
    VkRenderPass render_pass;
    u32 subpass;
};

typedef struct Shader_Code Shader_Code;
struct Shader_Code {
    u8 *data;
    u64 size;
};

Render_Pipeline render_pipeline_create(Arena *arena, Render_Context *rc,
                                       const char *vert_shader_path, const char *frag_shader_path,
                                       const Pipeline_Config *config);
void render_pipeline_free(Render_Context *rc, Render_Pipeline *pipeline);

// Sets all besides pipeline_layout, render_pass, and subpass
Pipeline_Config default_pipeline_config(u32 width, u32 height);

Shader_Code read_shader_file(Arena *arena, const char *file_path);
VkShaderModule create_shader_module(Shader_Code code, VkDevice device);

#endif // PIPELINE_H
