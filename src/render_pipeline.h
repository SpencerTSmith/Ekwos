#ifndef PIPELINE_H
#define PIPELINE_H

#include "common.h"
#include "render_context.h"

typedef struct Pipeline Pipeline;
struct Pipeline {
    VkPipelineLayout pipeline;
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

void create_graphics_pipeline(Arena *arena, Render_Context *rc, const char *vert_shader_path,
                              const char *frag_shader_path, Pipeline_Config config);

// Sets all besides pipeline_layout, render_pass, and subpass
Pipeline_Config default_pipeline_config(u32 width, u32 height);

Shader_Code read_shader_file(Arena *arena, const char *file_path);
VkShaderModule create_shader_module(Shader_Code code, VkDevice device);

#endif // PIPELINE_H
