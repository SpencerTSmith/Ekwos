#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include "core/arena.h"
#include "core/common.h"

#include <stdbool.h>

typedef struct Render_Pipeline Render_Pipeline;

// What validation layers, may move this out of here
extern const char *const enabled_validation_layers[];
extern const u32 num_enable_validation_layers;
extern const bool enable_val_layers;

// What device extensions do we need
extern const char *const device_extensions[];
extern const u32 num_device_extensions;

#define MAX_SWAP_IMGS 3
#define MAX_IN_FLIGHT (MAX_SWAP_IMGS - 1)
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
    VkSemaphore image_available_sem[MAX_IN_FLIGHT];
    VkSemaphore render_finished_sem[MAX_IN_FLIGHT];
    VkFence in_flight_fence[MAX_IN_FLIGHT];
    u32 curr_frame;
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
    VkCommandBuffer command_buffers[MAX_IN_FLIGHT];
};

#define VK_CHECK_FATAL(vk_func, message, ...)                                                      \
    do {                                                                                           \
        VkResult result = (vk_func);                                                               \
        if (result != VK_SUCCESS) {                                                                \
            LOG_FATAL(message, ##__VA_ARGS__);                                                     \
        }                                                                                          \
    } while (0)
#define VK_CHECK_ERROR(vk_func, message, ...)                                                      \
    do {                                                                                           \
        VkResult result = (vk_func);                                                               \
        if (result != VK_SUCCESS) {                                                                \
            LOG_ERROR(message, ##__VA_ARGS__);                                                     \
        }                                                                                          \
    } while (0)

// Can disable these messages at compile time
#ifdef DEBUG
#define VK_CHECK_WARN(vk_func, message, ...)                                                       \
    do {                                                                                           \
        VkResult result = (vk_func);                                                               \
        if (result != VK_SUCCESS) {                                                                \
            LOG_WARN(message, ##__VA_ARGS__);                                                      \
        }                                                                                          \
    } while (0)
#define VK_CHECK_DEBUG(vk_func, message, ...)                                                      \
    do {                                                                                           \
        VkResult result = (vk_func);                                                               \
        if (result != VK_SUCCESS) {                                                                \
            LOG_WARN(message, ##__VA_ARGS__);                                                      \
        }                                                                                          \
    } while (0)
#else
#define VK_CHECK_WARN(vk_func, message, ...)
#define VK_CHECK_DEBUG(vk_func, message, ...)
#endif

// TODO(spencer): Vulkan allows you to specify your own memory allocation function...         \
        // may want to incorporate this with our Arena allocator, or other custom allocator suited
// to it...

void render_context_init(Arena *arena, Render_Context *rndr_ctx, GLFWwindow *window_handle);
void render_context_free(Render_Context *rndr_ctx);

void render_record_command(Render_Context *rc, VkCommandBuffer buf, u32 image_idx,
                           Render_Pipeline *pipeline);
void render_frame(Render_Context *rc, Render_Pipeline *pipeline);

// Utility Functions //
u32 swap_height(Render_Context *rc);
u32 swap_width(Render_Context *rc);
#endif // RENDER_CONTEXT_H
