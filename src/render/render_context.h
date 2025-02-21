#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include "core/common.h"
#include "core/window.h"

#include "render/render_allocator.h"
#include "render/render_common.h"
#include "render/render_uploader.h"

#include <assert.h>
#include <stdbool.h>

enum RND_Context_Constants {
  RENDER_CONTEXT_MAX_SWAP_IMAGES = 3,
  RENDER_CONTEXT_MAX_FRAMES_IN_FLIGHT = 2,
  RENDER_CONTEXT_MAX_QUEUE_NUM = 2,
  RENDER_CONTEXT_MAX_PRESENT_MODES = 16,   // This could maybe change? I counted 7 in the enum
  RENDER_CONTEXT_MAX_SURFACE_FORMATS = 16, // no idea for this, made of 2 enums, lots of elems
  RENDER_CONTEXT_ATTACHMENT_COUNT = 2,
  RENDER_CONTEXT_STAGING_SIZE = MB(64),
};

// Just in case we want easy pointers
typedef struct RND_Swap_Chain RND_Swap_Chain;
typedef struct RND_Swap_Target RND_Swap_Target;
typedef struct RND_Swap_Frame RND_Swap_Frame;

typedef struct RND_Context RND_Context;
struct RND_Context {
  VkInstance instance;
  VkDebugUtilsMessengerEXT debug_messenger;
  VkSurfaceKHR surface;
  VkPhysicalDevice physical;

  VkDevice logical;
  VkQueue graphic_q;
  u32 graphic_index;
  VkQueue present_q;
  u32 present_index;

  RND_Allocator allocator;
  RND_Uploader uploader;

  // NOTE(ss): For now we group the render pass with the swap chain,
  // once I learn more this may not be the best practice
  struct RND_Swap_Chain {
    VkSwapchainKHR handle;

    VkClearDepthStencilValue clear_depth;
    VkClearColorValue clear_color;

    VkExtent2D extent;
    VkSurfaceFormatKHR surface_format;
    VkFormat depth_format;
    VkPresentModeKHR present_mode;

    VkRenderPass render_pass;
    u32 subpass;

    struct RND_Swap_Target {
      VkFramebuffer framebuffer;
      VkImage color_image;
      VkImageView color_image_view;
      VkImage depth_image;
      VkImageView depth_image_view;
      VkDeviceMemory depth_memory;
    } targets[RENDER_CONTEXT_MAX_SWAP_IMAGES];
    u32 current_target_idx;
    u32 target_count;

    VkCommandPool command_pool;
    struct RND_Swap_Frame {
      VkCommandBuffer command_buffer;
      VkSemaphore image_available_sem;
      VkSemaphore render_finished_sem;
      VkFence in_flight_fence;
    } frames[RENDER_CONTEXT_MAX_FRAMES_IN_FLIGHT];
    u32 current_frame_idx;
    u32 frames_in_flight;
  } swap;
};

// TODO(spencer): Vulkan allows you to specify your own memory allocation function...
// may want to incorporate this with our Arena allocator, or other custom allocator suited
// to it...

void rnd_context_init(RND_Context *render_context, Window *window);
void rnd_context_free(RND_Context *render_context);

void rnd_begin_frame(RND_Context *render_context, Window *window);
void rnd_end_frame(RND_Context *render_context);

// Utility Functions //
u32 rnd_swap_height(const RND_Context *render_context);
u32 rnd_swap_width(const RND_Context *render_context);
f32 rnd_swap_aspect_ratio(const RND_Context *render_context);

const RND_Swap_Frame *rnd_get_current_frame(const RND_Context *rc);
VkCommandBuffer rnd_get_current_cmd(const RND_Context *rc);

#endif // RENDER_CONTEXT_H
