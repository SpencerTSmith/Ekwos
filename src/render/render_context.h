#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include "core/arena.h"
#include "core/common.h"
#include "core/log.h"
#include "window.h"

#include <stdbool.h>

// Taken from Vulkan Samples, with couple modifications , the fatal version will exit with specified
// code for you
#define VK_CHECK_FATAL(x, exit_code, message, ...)                                                 \
    do {                                                                                           \
        VkResult check = x;                                                                        \
        if (check != VK_SUCCESS) {                                                                 \
            LOG_FATAL(message, ##__VA_ARGS__);                                                     \
            exit(exit_code);                                                                       \
        }                                                                                          \
    } while (0)

#define VK_CHECK_ERROR(x, message, ...)                                                            \
    do {                                                                                           \
        VkResult check = x;                                                                        \
        if (check != VK_SUCCESS) {                                                                 \
            LOG_ERROR(message, ##__VA_ARGS__);                                                     \
        }                                                                                          \
    } while (0)

// Constants
enum {
    MAX_SWAP_IMGS = 3,
    MAX_FRAMES_IN_FLIGHT = 2,
    QUEUE_NUM = 2,
};

typedef struct Swap_Chain Swap_Chain; // Just in case we want easy pointers
typedef struct Render_Context Render_Context;
struct Render_Context {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_msgr;
    VkSurfaceKHR surface;

    VkPhysicalDevice physical;
    VkDevice logical;
    VkQueue graphic_q;
    u32 graphic_index;
    VkQueue present_q;
    u32 present_index;

    // NOTE(ss): For now we group the render pass with the swap chain,
    // once I learn more this may not be the best practice
    struct Swap_Chain {
        VkSwapchainKHR handle;
        VkExtent2D extent;
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;
        VkFramebuffer framebuffers[MAX_SWAP_IMGS];
        VkImage images[MAX_SWAP_IMGS];
        VkImageView image_views[MAX_SWAP_IMGS];
        VkImage depth_images[MAX_SWAP_IMGS];
        VkImageView depth_image_views[MAX_SWAP_IMGS];
        u32 image_count;
        u32 curr_image_idx;
        VkRenderPass render_pass;
        u32 subpass;
        VkCommandPool command_pool;
        // TODO(ss): Really think this out, we want all these resources to only ever be grouped with
        // their fellows in the same frame... how best to do this? Struct, have a reference to the
        // current struct? Or keep the curr_frame idx? Helper function?
        VkCommandBuffer command_buffers[MAX_FRAMES_IN_FLIGHT];
        VkSemaphore image_available_sem[MAX_FRAMES_IN_FLIGHT];
        VkSemaphore render_finished_sem[MAX_FRAMES_IN_FLIGHT];
        VkFence in_flight_fence[MAX_FRAMES_IN_FLIGHT];
        u32 curr_frame;
    } swap;
};

// TODO(spencer): Vulkan allows you to specify your own memory allocation function...
// may want to incorporate this with our Arena allocator, or other custom allocator suited
// to it...

void render_context_init(Arena *arena, Render_Context *render_context, GLFWwindow *window_handle);
void render_context_free(Render_Context *render_context);

bool render_begin_frame(Render_Context *render_context, Window *window);
void render_end_frame(Render_Context *render_context);

// Utility Functions //
u32 render_swap_height(Render_Context *render_context);
u32 render_swap_width(Render_Context *render_context);

#endif // RENDER_CONTEXT_H
