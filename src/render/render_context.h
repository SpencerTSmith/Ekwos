#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include "core/arena.h"
#include "core/common.h"
#include "core/log.h"
#include "window.h"

#include <assert.h>
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
    MAX_SWAP_IMAGES = 3,
    MAX_FRAMES_IN_FLIGHT = 2,
    QUEUE_NUM = 2,
};

// Just in case we want easy pointers
typedef struct Swap_Chain Swap_Chain;
typedef struct Swap_Target Swap_Target;
typedef struct Swap_Frame Swap_Frame;

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

        VkClearColorValue clear_color;

        VkExtent2D extent;
        VkSurfaceFormatKHR surface_format;
        VkPresentModeKHR present_mode;

        VkRenderPass render_pass;
        u32 subpass;

        struct Swap_Target {
            VkFramebuffer framebuffer;
            VkImage image;
            VkImageView image_view;
        } targets[MAX_SWAP_IMAGES];
        u32 current_target_idx;
        u32 target_count;

        VkCommandPool command_pool;
        struct Swap_Frame {
            VkCommandBuffer command_buffer;
            VkSemaphore image_available_sem;
            VkSemaphore render_finished_sem;
            VkFence in_flight_fence;
        } frames[MAX_FRAMES_IN_FLIGHT];
        u32 current_frame_idx;
        u32 frames_in_flight;
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
u32 render_get_swap_height(const Render_Context *render_context);
u32 render_get_swap_width(const Render_Context *render_context);
static inline const Swap_Frame *render_get_current_frame(const Render_Context *rc) {
    assert(rc != NULL);
    return &rc->swap.frames[rc->swap.current_frame_idx];
}
static inline VkCommandBuffer render_get_current_command(const Render_Context *rc) {
    return render_get_current_frame(rc)->command_buffer;
}

#endif // RENDER_CONTEXT_H
