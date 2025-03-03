#ifndef PIPELINE_H
#define PIPELINE_H

#include "core/common.h"
#include "render/render_context.h"

typedef struct RND_Pipeline RND_Pipeline;
struct RND_Pipeline {
  VkPipeline handle;
  VkPipelineLayout layout;
};

enum {
  RND_PIPELINE_MAX_DYNAMIC_PIPELINE_STATES = 2,
};

typedef struct Pipeline_Config Pipeline_Config;
struct Pipeline_Config {
  VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
  VkPipelineRasterizationStateCreateInfo rasterization_info;
  VkPipelineMultisampleStateCreateInfo multisample_info;
  VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
  VkPipelineViewportStateCreateInfo viewport_info;
  VkPipelineColorBlendAttachmentState color_blend_attachment_state;
  VkDynamicState dynamic_states[RND_PIPELINE_MAX_DYNAMIC_PIPELINE_STATES];
  u32 dynamic_state_count;
};

// Remember alignment shit
typedef struct RND_Push_Constants RND_Push_Constants;
struct RND_Push_Constants {
  mat4 clip_transform;
  mat4 normal_matrix; // For some reason only works when its a mat4?!
};

// Will use a default configuration if NULL passed in for config parameter
RND_Pipeline rnd_pipeline_make(RND_Context *rc, const char *vert_shader_path,
                               const char *frag_shader_path, const Pipeline_Config *config);
void rnd_pipeline_free(RND_Context *render_context, RND_Pipeline *pipeline);

void rnd_pipeline_bind(RND_Context *render_context, RND_Pipeline *pipeline);

void rnd_pipeline_push_constants(RND_Context *rc, RND_Pipeline *pl, RND_Push_Constants push);

#endif // PIPELINE_H
