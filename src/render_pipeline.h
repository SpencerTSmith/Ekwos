#ifndef PIPELINE_H
#define PIPELINE_H

#include "common.h"
#include "render_context.h"

typedef struct Render_Pipeline Render_Pipeline;
struct Render_Pipeline {
    VkPipeline handle;
};

typedef struct Pipeline_Config Pipeline_Config;
struct Pipeline_Config {
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineViewportStateCreateInfo viewport_info;
    VkPipelineRasterizationStateCreateInfo rasterization_info;
    VkPipelineMultisampleStateCreateInfo multisample_info;
    VkPipelineColorBlendAttachmentState color_blend_attachment;
    VkPipelineColorBlendStateCreateInfo color_blend_info;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
    VkPipelineLayout pipeline_layout;
    VkRenderPass render_pass;
    u32 subpass;
};

typedef struct Shader_Code Shader_Code;
struct Shader_Code {
    u8 *data;
    u64 size;
};

Render_Pipeline graphics_pipeline_create(Arena *arena, Render_Context *rc,
                                         const char *vert_shader_path, const char *frag_shader_path,
                                         const Pipeline_Config *config);
void graphics_pipeline_free(Render_Context *rc, Render_Pipeline *pipeline);

// Sets all besides pipeline_layout, render_pass, and subpass
void default_pipeline_config(Pipeline_Config *config, u32 width, u32 height);

Shader_Code read_shader_file(Arena *arena, const char *file_path);
VkShaderModule create_shader_module(Shader_Code code, VkDevice device);

#endif // PIPELINE_H
