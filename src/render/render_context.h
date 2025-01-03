#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include "core/arena.h"
#include "core/common.h"

#include <stdbool.h>

// What validation layers, may move this out of here
extern const char *const enabled_validation_layers[];
extern const u32 num_enable_validation_layers;
extern const bool enable_val_layers;

// What device extensions do we need
extern const char *const device_extensions[];
extern const u32 num_device_extensions;

#define MAX_SWAP_IMGS 3
typedef struct Swap_Chain Swap_Chain;
struct Swap_Chain {
    VkSwapchainKHR handle;
    VkFormat format;
    VkExtent2D extent;
    VkFramebuffer framebuffers[MAX_SWAP_IMGS];
    VkImage images[MAX_SWAP_IMGS];
    VkImageView image_views[MAX_SWAP_IMGS];
    VkImage depth_images[MAX_SWAP_IMGS];
    VkImageView depth_image_views[MAX_SWAP_IMGS];
    u32 image_count;
    VkRenderPass render_pass;
    u32 subpass;
    VkSemaphore image_available_sem;
    VkSemaphore render_finished_sem;
    VkFence in_flight_fence;
};

typedef struct Swap_Chain_Info Swap_Chain_Info;
struct Swap_Chain_Info {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    u32 format_count;
    VkPresentModeKHR *present_modes;
    u32 present_mode_count;
};

#define QUEUE_NUM 2
typedef struct Queue_Family_Indices Queue_Family_Indices;
struct Queue_Family_Indices {
    u32 graphic;
    u32 present;
};

typedef struct Render_Context Render_Context;
struct Render_Context {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physical;
    VkDevice logical;
    VkQueue graphic_q;
    u32 graphic_index;
    VkQueue present_q;
    u32 present_index;
    Swap_Chain swap;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
};

typedef struct Render_Pipeline Render_Pipeline;
// TODO(spencer): Vulkan allows you to specify your own memory allocation function...
// may want to incorporate this with our Arena allocator, or other custom allocator suited to it...

void render_context_init(Arena *arena, Render_Context *rndr_ctx, GLFWwindow *window_handle);
void render_context_free(Render_Context *rndr_ctx);

void render_record_command(Render_Context *rc, VkCommandBuffer buf, u32 image_idx,
                           Render_Pipeline *pipeline);
void render_frame(Render_Context *rc, Render_Pipeline *pipeline);

// Utility Functions //
u32 swap_height(Render_Context *rc);
u32 swap_width(Render_Context *rc);
#endif // RENDER_CONTEXT_H
