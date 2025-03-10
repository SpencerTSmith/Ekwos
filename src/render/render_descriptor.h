#ifndef RENDER_DESCRIPTOR
#define RENDER_DESCRIPTOR

#include "core/common.h"
#include "render/render_common.h"

enum RND_Descriptor_Constants {
  RND_DESCRIPTOR_MAX_BINDINGS = 5,
};

struct RND_Descriptor {
  VkDescriptorSetLayoutBinding bindings[RND_DESCRIPTOR_MAX_BINDINGS];
  u32 bindings_count;

  VkDescriptorSetLayout layout;
};

struct RND_Descriptor_Pool {
  VkDescriptorPool pool;
};

#endif // RENDER_DESCRIPTOR
