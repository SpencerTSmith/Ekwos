#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include "core/arena.h"
#include "core/common.h"
#include "core/log.h"
#include "render/render_allocator.h"
#include "render/render_common.h"
#include "window.h"

#include <assert.h>
#include <stdbool.h>

enum Render_Context_Constants {
    RENDER_CONTEXT_MAX_SWAP_IMAGES = 3,
    RENDER_CONTEXT_MAX_FRAMES_IN_FLIGHT = 2,
    RENDER_CONTEXT_MAX_QUEUE_NUM = 2,
    RENDER_CONTEXT_MAX_PRESENT_MODES = 16,   // This could maybe change? I counted 7 in the enum
    RENDER_CONTEXT_MAX_SURFACE_FORMATS = 16, // no idea for this, made of 2 enums, lots of elems
    RENDER_CONTEXT_ATTACHMENT_COUNT = 2,
};

// Just in case we want easy pointers
typedef struct Swap_Chain Swap_Chain;
typedef struct Swap_Target Swap_Target;
typedef struct Swap_Frame Swap_Frame;

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

    // NOTE(ss): For now we group the render pass with the swap chain,
    // once I learn more this may not be the best practice
    struct Swap_Chain {
        VkSwapchainKHR handle;

        VkClearDepthStencilValue clear_depth;
        VkClearColorValue clear_color;

        VkExtent2D extent;
        VkSurfaceFormatKHR surface_format;
        VkFormat depth_format;
        VkPresentModeKHR present_mode;

        VkRenderPass render_pass;
        u32 subpass;

        Render_Arena arena;
        struct Swap_Target {
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
        struct Swap_Frame {
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

void render_context_init(Arena *arena, Render_Context *render_context, GLFWwindow *window_handle);
void render_context_free(Render_Context *render_context);

void render_begin_frame(Render_Context *render_context, Window *window);
void render_end_frame(Render_Context *render_context);

// Utility Functions //
u32 render_get_swap_height(const Render_Context *render_context);
u32 render_get_swap_width(const Render_Context *render_context);

static inline const Swap_Frame *render_get_current_frame(const Render_Context *rc) {
    assert(rc != NULL);
    return &rc->swap.frames[rc->swap.current_frame_idx];
}
static inline VkCommandBuffer render_get_current_cmd(const Render_Context *rc) {
    return render_get_current_frame(rc)->command_buffer;
}
// static inline const Swap_Target *render_get_current_target(const Render_Context *rc) {
//     assert(rc != NULL);
//     return &rc->swap.targets[rc->swap.current_target_idx];
// }

#endif // RENDER_CONTEXT_H
