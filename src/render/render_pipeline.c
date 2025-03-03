#include "render/render_pipeline.h"

#include "core/log.h"
#include "core/thread_context.h"
#include "render/render_mesh.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Shader_Code Shader_Code;
struct Shader_Code {
  u8 *data;
  u64 size;
};

// Sets all besides pipeline_layout, render_pass, and subpass
translation_local Pipeline_Config default_pipeline_config(void);

translation_local Shader_Code read_shader_file(Arena *arena, const char *file_path);
translation_local VkShaderModule create_shader_module(Shader_Code code, VkDevice device);

RND_Pipeline rnd_pipeline_make(RND_Context *rc, const char *vert_shader_path,
                               const char *frag_shader_path, const Pipeline_Config *config) {
  // Use a default if none passed in
  Pipeline_Config pl_config = config == NULL ? default_pipeline_config() : *config;

  // Don't need to keep the shader code memory around
  Scratch scratch = thread_get_scratch();

  RND_Pipeline pipeline = {0};
  Shader_Code vert_code = read_shader_file(scratch.arena, vert_shader_path);
  Shader_Code frag_code = read_shader_file(scratch.arena, frag_shader_path);
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
  vertex_input_info.vertexAttributeDescriptionCount = RND_VERTEX_ATTRIBUTES_NUM;
  vertex_input_info.pVertexAttributeDescriptions = g_rnd_vertex_attrib_descs;
  vertex_input_info.vertexBindingDescriptionCount = RND_VERTEX_BINDINGS_NUM;
  vertex_input_info.pVertexBindingDescriptions = g_rnd_vertex_binding_descs;

  VkPipelineColorBlendStateCreateInfo color_blend_info = {0};
  color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  color_blend_info.logicOpEnable = VK_FALSE;
  color_blend_info.logicOp = VK_LOGIC_OP_COPY;
  color_blend_info.attachmentCount = 1;
  color_blend_info.pAttachments = &pl_config.color_blend_attachment_state;
  color_blend_info.blendConstants[0] = 0.0f;
  color_blend_info.blendConstants[1] = 0.0f;
  color_blend_info.blendConstants[2] = 0.0f;
  color_blend_info.blendConstants[3] = 0.0f;

  VkPipelineDynamicStateCreateInfo dynamic_state_info = {0};
  dynamic_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamic_state_info.dynamicStateCount = pl_config.dynamic_state_count;
  dynamic_state_info.pDynamicStates = pl_config.dynamic_states;

  VkPushConstantRange push_constants_range = {0};
  push_constants_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  push_constants_range.offset = 0;
  push_constants_range.size = sizeof(RND_Push_Constants);

  VkPipelineLayoutCreateInfo layout_info = {0};
  layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  layout_info.setLayoutCount = 0;
  layout_info.pSetLayouts = NULL;
  layout_info.pushConstantRangeCount = 1;
  layout_info.pPushConstantRanges = &push_constants_range;

  VK_CHECK_FATAL(vkCreatePipelineLayout(rc->logical, &layout_info, NULL, &pipeline.layout),
                 EXT_VK_PIPELINE_LAYOUT, "Failed to create pipeline layout");

  VkGraphicsPipelineCreateInfo pipeline_info = {0};
  pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

  // Every thing we finish creating / configuring after getting the config structure
  pipeline_info.stageCount = 2;
  pipeline_info.pStages = shader_stages;
  pipeline_info.pColorBlendState = &color_blend_info;
  pipeline_info.pDynamicState = &dynamic_state_info;
  pipeline_info.pVertexInputState = &vertex_input_info;

  // Enough info from our struct
  pipeline_info.pInputAssemblyState = &pl_config.input_assembly_info;
  pipeline_info.pViewportState = &pl_config.viewport_info;
  pipeline_info.pMultisampleState = &pl_config.multisample_info;
  pipeline_info.pRasterizationState = &pl_config.rasterization_info;
  pipeline_info.pDepthStencilState = &pl_config.depth_stencil_info;

  // Stuff we keep track of either from the general render context or with this specific pipeline
  pipeline_info.layout = pipeline.layout;
  pipeline_info.renderPass = rc->swap.render_pass;
  pipeline_info.subpass = rc->swap.subpass;

  // If we want to inherit state from another pipeline
  pipeline_info.basePipelineIndex = -1;
  pipeline_info.basePipelineHandle = VK_NULL_HANDLE;

  VK_CHECK_FATAL(vkCreateGraphicsPipelines(rc->logical, VK_NULL_HANDLE, 1, &pipeline_info, NULL,
                                           &pipeline.handle),
                 EXT_VK_PIPELINE_CREATE, "Failed to create pipeline");

  // We can clean up any shader modules now
  vkDestroyShaderModule(rc->logical, vert_mod, NULL);
  vkDestroyShaderModule(rc->logical, frag_mod, NULL);

  LOG_DEBUG("Render Pipeline resources initialized");
  return pipeline;
}

void rnd_pipeline_free(RND_Context *rc, RND_Pipeline *pl) {
  vkDestroyPipelineLayout(rc->logical, pl->layout, NULL);
  vkDestroyPipeline(rc->logical, pl->handle, NULL);
  ZERO_STRUCT(pl);

  LOG_DEBUG("Render Pipeline resources destroyed");
}

void rnd_pipeline_bind(RND_Context *rc, RND_Pipeline *pl) {
  vkCmdBindPipeline(rnd_get_current_cmd(rc), VK_PIPELINE_BIND_POINT_GRAPHICS, pl->handle);
}

void rnd_pipeline_push_constants(RND_Context *rc, RND_Pipeline *pl, RND_Push_Constants push) {
  vkCmdPushConstants(rnd_get_current_cmd(rc), pl->layout,
                     VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                     sizeof(RND_Push_Constants), &push);
}

translation_local Pipeline_Config default_pipeline_config(void) {
  Pipeline_Config config = {0};
  // What is the primitive assembly like? (How are vertices treated... triangles, points, etc)
  config.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  config.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  config.input_assembly_info.primitiveRestartEnable = VK_FALSE;

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

  config.color_blend_attachment_state.colorWriteMask =
      VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
      VK_COLOR_COMPONENT_A_BIT;
  config.color_blend_attachment_state.blendEnable = VK_FALSE;
  config.color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
  config.color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
  config.color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
  config.color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  config.color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

  config.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  config.depth_stencil_info.depthTestEnable = VK_TRUE;
  config.depth_stencil_info.depthWriteEnable = VK_TRUE;
  config.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
  config.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
  config.depth_stencil_info.minDepthBounds = 0.0f;
  config.depth_stencil_info.maxDepthBounds = 1.0f;
  config.depth_stencil_info.stencilTestEnable = VK_FALSE;
  config.depth_stencil_info.front = (VkStencilOpState){0};
  config.depth_stencil_info.back = (VkStencilOpState){0};

  // Dynamic yo
  config.viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  config.viewport_info.viewportCount = 1;
  config.viewport_info.pViewports = VK_NULL_HANDLE;
  config.viewport_info.scissorCount = 1;
  config.viewport_info.pScissors = VK_NULL_HANDLE;

  config.dynamic_states[0] = VK_DYNAMIC_STATE_VIEWPORT;
  config.dynamic_states[1] = VK_DYNAMIC_STATE_SCISSOR;
  config.dynamic_state_count = 2;

  return config;
}

translation_local Shader_Code read_shader_file(Arena *arena, const char *file_path) {
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

  i64 byte_count = ftell(shader_file);
  if (byte_count == -1L) {
    LOG_ERROR("Failed to get size of shader file: %s, %s\n", file_path, strerror(errno));
    fclose(shader_file);
    return shader_data;
  }

  shader_data.size = byte_count;
  rewind(shader_file);

  // aligned with u32, since that is what vulkan is expecting
  shader_data.data = arena_calloc(arena, byte_count, u8);

  i64 bytes_read = fread(shader_data.data, 1, byte_count, shader_file);
  if (bytes_read != byte_count) {
    LOG_ERROR("Bytes read from shader file does not match shader size: %s, %s\n", file_path,
              strerror(errno));
    arena_pop(arena, byte_count);
    fclose(shader_file);
    return shader_data;
  }

  fclose(shader_file);
  LOG_DEBUG("Read shader file: %s, with size %u", file_path, byte_count);
  return shader_data;
}

translation_local VkShaderModule create_shader_module(Shader_Code code, VkDevice device) {
  VkShaderModuleCreateInfo ci = {0};
  ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  ci.codeSize = code.size;
  // Pointer cast, vulkan wants u32
  ci.pCode = (const u32 *)code.data;

  VkShaderModule shader_module;
  VK_CHECK_FATAL(vkCreateShaderModule(device, &ci, NULL, &shader_module), EXT_VK_SHADER_MODULE,
                 "Failed to create shader module");

  return shader_module;
}
