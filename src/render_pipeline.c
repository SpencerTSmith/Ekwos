#include "render_pipeline.h"

#include <errno.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void create_graphics_pipeline(Arena *arena, Render_Context *rc, const char *vert_shader_path,
                              const char *frag_shader_path, Pipeline_Config config) {
    // Don't need to keep the shader code memory around
    Scratch scratch = scratch_begin(arena);
    Shader_Code vert_code = read_shader_file(arena, vert_shader_path);
    Shader_Code frag_code = read_shader_file(arena, frag_shader_path);
    VkShaderModule vert_mod = create_shader_module(vert_code, rc->logical.handle);
    VkShaderModule frag_mod = create_shader_module(frag_code, rc->logical.handle);
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
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = VK_NULL_HANDLE;
    vertex_input_info.pVertexBindingDescriptions = VK_NULL_HANDLE;
}

Pipeline_Config default_pipeline_config(u32 width, u32 height) {
    // What is the primitive assembly like? (How are vertices treated... triangles, points, etc)
    Pipeline_Config c = {0};
    c.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_INFO_KHR;
    c.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    c.input_assembly_info.primitiveRestartEnable = VK_FALSE;

    // How do we go from NDC to screen coords?
    c.viewport.x = 0.0f;
    c.viewport.y = 0.0f;
    c.viewport.width = width;
    c.viewport.height = height;
    c.viewport.minDepth = 0.0f;
    c.viewport.maxDepth = 1.0f;

    c.scissor.offset = (VkOffset2D){0, 0};
    c.scissor.extent = (VkExtent2D){width, height};

    // Yet more bullshit we have to create
    c.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    c.viewport_info.viewportCount = 1;
    c.viewport_info.pViewports = &c.viewport;
    c.viewport_info.scissorCount = 1;
    c.viewport_info.pScissors = &c.scissor;

    c.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    c.rasterization_info.depthClampEnable = VK_FALSE;
    c.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    c.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    c.rasterization_info.lineWidth = 1.0f;
    c.rasterization_info.cullMode = VK_CULL_MODE_NONE;
    c.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    c.rasterization_info.depthBiasEnable = VK_FALSE;
    c.rasterization_info.depthBiasConstantFactor = 0.0f;
    c.rasterization_info.depthBiasClamp = 0.0f;
    c.rasterization_info.depthBiasSlopeFactor = 0.0f;

    c.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    c.multisample_info.sampleShadingEnable = VK_FALSE;
    c.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    c.multisample_info.minSampleShading = 1.0f;
    c.multisample_info.pSampleMask = VK_NULL_HANDLE;
    c.multisample_info.alphaToCoverageEnable = VK_FALSE;
    c.multisample_info.alphaToOneEnable = VK_FALSE;

    c.color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    c.color_blend_attachment.blendEnable = VK_FALSE;
    c.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    c.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    c.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    c.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    c.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    c.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    c.color_blend_info.logicOpEnable = VK_FALSE;
    c.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
    c.color_blend_info.attachmentCount = 1;
    c.color_blend_info.pAttachments = &c.color_blend_attachment;
    c.color_blend_info.blendConstants[0] = 0.0f;
    c.color_blend_info.blendConstants[1] = 0.0f;
    c.color_blend_info.blendConstants[2] = 0.0f;
    c.color_blend_info.blendConstants[3] = 0.0f;

    return c;
}

Shader_Code read_shader_file(Arena *arena, const char *file_path) {
    Shader_Code shader_data = {0};

    FILE *shader_file = fopen(file_path, "rb");
    if (shader_file == NULL) {
        fprintf(stderr, "Error (%s) : Failed to read shader file: %s\n", strerror(errno),
                file_path);
        return shader_data;
    }

    if (fseek(shader_file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error (%s) : Failed to seek end of file: %s\n", strerror(errno),
                file_path);
        fclose(shader_file);
        return shader_data;
    }

    i64 size = ftell(shader_file);
    if (size == -1L) {
        fprintf(stderr, "Error (%s) : Failed to get size of file: %s\n", strerror(errno),
                file_path);
        fclose(shader_file);
        return shader_data;
    }

    shader_data.size = size;
    rewind(shader_file);

    // aligned with u32, since that is what vulkan is expecting
    shader_data.data = arena_alloc(arena, size * sizeof(shader_data.data), alignof(u32));

    u64 bytes_read = fread(shader_data.data, 1, size, shader_file);
    if (bytes_read != size) {
        fprintf(stderr, "Error (%s) : shader bytes read into buffer does not match file size: %s\n",
                strerror(errno), file_path);
        arena_pop(arena, size);
        fclose(shader_file);
        return shader_data;
    }

    fclose(shader_file);
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
        fprintf(stderr, "Failed to create shader module\n");
    }

    return shader_module;
}
