#include "render_pipeline.h"
#include "core/log.h"
#include "render/render_model.h"

#include <errno.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Render_Pipeline render_pipeline_create(Arena *arena, Render_Context *rc,
                                       const char *vert_shader_path, const char *frag_shader_path,
                                       const Pipeline_Config *config) {
    Render_Pipeline pipeline = {0};
    // Don't need to keep the shader code memory around
    Scratch scratch = scratch_begin(arena);
    Shader_Code vert_code = read_shader_file(arena, vert_shader_path);
    Shader_Code frag_code = read_shader_file(arena, frag_shader_path);
    VkShaderModule vert_mod = create_shader_module(vert_code, rc->logical);
    VkShaderModule frag_mod = create_shader_module(frag_code, rc->logical);
    scratch_end(&scratch);

    VkPipelineShaderStageCreateInfo shader_stages[2] = {0};
    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = vert_mod;
    shader_stages[0].pName = "main";

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = frag_mod;
    shader_stages[1].pName = "main";

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = VERTEX_ATTRIBUTES_NUM;
    vertex_input_info.vertexBindingDescriptionCount = VERTEX_BINDING_NUM;
    vertex_input_info.pVertexAttributeDescriptions = vertex_attrib_desc;
    vertex_input_info.pVertexBindingDescriptions = vertex_binding_desc;

    VkPipelineColorBlendStateCreateInfo color_blend_info = {0};
    color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_info.logicOpEnable = VK_FALSE;
    color_blend_info.logicOp = VK_LOGIC_OP_COPY;
    color_blend_info.attachmentCount = 1;
    color_blend_info.pAttachments = &config->color_blend_attachment;
    color_blend_info.blendConstants[0] = 0.0f;
    color_blend_info.blendConstants[1] = 0.0f;
    color_blend_info.blendConstants[2] = 0.0f;
    color_blend_info.blendConstants[3] = 0.0f;

    // Yet more bullshit we have to create
    VkPipelineViewportStateCreateInfo viewport_info = {0};
    viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_info.viewportCount = 1;
    viewport_info.pViewports = &config->viewport;
    viewport_info.scissorCount = 1;
    viewport_info.pScissors = &config->scissor;

    VkResult result = vkCreatePipelineLayout(rc->logical, &config->pipeline_layout_info, NULL,
                                             &pipeline.pipeline_layout);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &config->input_assembly_info;
    pipeline_info.pViewportState = &viewport_info;
    pipeline_info.pMultisampleState = &config->multisample_info;
    pipeline_info.pRasterizationState = &config->rasterization_info;
    pipeline_info.pDepthStencilState = &config->depth_stencil_info;
    pipeline_info.pColorBlendState = &color_blend_info;
    pipeline_info.pDynamicState = NULL;

    pipeline_info.layout = pipeline.pipeline_layout;
    pipeline_info.renderPass = rc->swap.render_pass;
    pipeline_info.subpass = rc->swap.subpass;

    // If we want to inherit state from another pipeline
    pipeline_info.basePipelineIndex = -1;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

    result = vkCreateGraphicsPipelines(rc->logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL,
                                       &pipeline.handle);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create pipeline");
    }

    // We can clean up any shader modules now
    vkDestroyShaderModule(rc->logical, vert_mod, NULL);
    vkDestroyShaderModule(rc->logical, frag_mod, NULL);

    LOG_DEBUG("Render Pipeline resources initialized");
    return pipeline;
}

void render_pipeline_free(Render_Context *rc, Render_Pipeline *pipeline) {
    vkDestroyPipelineLayout(rc->logical, pipeline->pipeline_layout, NULL);
    vkDestroyPipeline(rc->logical, pipeline->handle, NULL);
    memset(pipeline, 0, sizeof(*pipeline));
    LOG_DEBUG("Render Pipeline resources destroyed");
}

void render_pipeline_bind(Render_Context *rc, Render_Pipeline *pipeline) {
    vkCmdBindPipeline(rc->command_buffers[rc->curr_frame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline->handle);
}

Pipeline_Config default_pipeline_config(u32 width, u32 height) {
    Pipeline_Config config = {0};
    // What is the primitive assembly like? (How are vertices treated... triangles, points, etc)
    config.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.input_assembly_info.primitiveRestartEnable = VK_FALSE;

    // How do we go from NDconfig.to screen coords?
    config.viewport.x = 0.0f;
    config.viewport.y = 0.0f;
    config.viewport.width = width;
    config.viewport.height = height;
    config.viewport.minDepth = 0.0f;
    config.viewport.maxDepth = 1.0f;

    VkOffset2D offset = {.x = 0, .y = 0};
    VkExtent2D extent = {.width = width, .height = height};
    config.scissor.offset = offset;
    config.scissor.extent = extent;

    config.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config.rasterization_info.depthClampEnable = VK_FALSE;
    config.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    config.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    config.rasterization_info.lineWidth = 1.0f;
    config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
    config.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    config.rasterization_info.depthBiasEnable = VK_FALSE;
    config.rasterization_info.depthBiasConstantFactor = 0.0f;
    config.rasterization_info.depthBiasClamp = 0.0f;
    config.rasterization_info.depthBiasSlopeFactor = 0.0f;

    config.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config.multisample_info.sampleShadingEnable = VK_FALSE;
    config.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    config.multisample_info.minSampleShading = 1.0f;
    config.multisample_info.pSampleMask = VK_NULL_HANDLE;
    config.multisample_info.alphaToCoverageEnable = VK_FALSE;
    config.multisample_info.alphaToOneEnable = VK_FALSE;

    config.color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    config.color_blend_attachment.blendEnable = VK_FALSE;
    config.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    config.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    config.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    config.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    config.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    config.pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    config.pipeline_layout_info.setLayoutCount = 0;
    config.pipeline_layout_info.pSetLayouts = NULL;
    config.pipeline_layout_info.pushConstantRangeCount = 0;
    config.pipeline_layout_info.pPushConstantRanges = NULL;

    return config;
}

Shader_Code read_shader_file(Arena *arena, const char *file_path) {
    Shader_Code shader_data = {0};

    FILE *shader_file = fopen(file_path, "rb");
    if (shader_file == NULL) {
        LOG_ERROR("Failed to read shader file: %s, %s\n", file_path, strerror(errno));
        return shader_data;
    }

    if (fseek(shader_file, 0, SEEK_END) != 0) {
        LOG_ERROR("Failed to find end of shader file: %s, %s\n", file_path, strerror(errno));
        fclose(shader_file);
        return shader_data;
    }

    i64 size = ftell(shader_file);
    if (size == -1L) {
        LOG_ERROR("Failed to get size of shader file: %s, %s\n", file_path, strerror(errno));
        fclose(shader_file);
        return shader_data;
    }

    shader_data.size = size;
    rewind(shader_file);

    // aligned with u32, since that is what vulkan is expecting
    shader_data.data = arena_alloc(arena, size * sizeof(shader_data.data), alignof(u32));

    i64 bytes_read = fread(shader_data.data, 1, size, shader_file);
    if (bytes_read != size) {
        LOG_ERROR("Bytes read from shader file does not match shader size: %s, %s\n", file_path,
                  strerror(errno));
        arena_pop(arena, size);
        fclose(shader_file);
        return shader_data;
    }

    fclose(shader_file);
    LOG_DEBUG("Read shader file: %s, with size %u", file_path, size);
    return shader_data;
}

VkShaderModule create_shader_module(Shader_Code code, VkDevice device) {
    VkShaderModuleCreateInfo ci = {0};
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = code.size;
    // Pointer cast, vulkan wants u32
    ci.pCode = (const u32 *)code.data;

    VkShaderModule shader_module;
    if (vkCreateShaderModule(device, &ci, NULL, &shader_module)) {
        LOG_ERROR("Failed to create shader module");
    }

    return shader_module;
}
