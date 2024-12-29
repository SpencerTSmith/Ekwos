#ifndef RENDER_CONTEXT_H
#define RENDER_CONTEXT_H

#include "arena.h"
#include "common.h"

#include <stdbool.h>

// What validation layers, may move this out of here
extern const char *const enabled_validation_layers[];
extern const u32 num_enable_validation_layers;
extern const bool enable_val_layers;

// What device extensions do we need
extern const char *const device_extensions[];
extern const u32 num_device_extensions;

// Contains device and it's queues
typedef struct Logical_Device Logical_Device;
struct Logical_Device {
    VkDevice handle;
    VkQueue graphic_q;
    VkQueue present_q;
};

#define MAX_SWAP_IMGS 3
typedef struct Swap_Chain Swap_Chain;
struct Swap_Chain {
    VkSwapchainKHR handle;
    VkFormat img_format;
    VkExtent2D extent;
    VkImage imgs[MAX_SWAP_IMGS];
    VkImageView img_views[MAX_SWAP_IMGS];
    u32 imgs_count;
};

typedef struct Swap_Chain_Info Swap_Chain_Info;
struct Swap_Chain_Info {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    u32 num_formats;
    VkPresentModeKHR *present_modes;
    u32 num_present_modes;
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
    Logical_Device logical;
    Swap_Chain swap;
};

// TODO(spencer): Vulkan allows you to specify your own memory allocation function...
// may want to incorporate this with our Arena allocator, or other custom allocator suited to it...

void render_context_init(Arena *arena, Render_Context *rndr_ctx, GLFWwindow *window_handle);
void render_context_free(Render_Context *rndr_ctx);

// This also sets up our debug messenger if it's needed
void create_instance(Arena *arena, Render_Context *rndr_ctx);

bool check_val_layer_support(Arena *arena, const char *const *layers, u32 num_layers);
const char **get_glfw_required_extensions(Arena *arena, u32 *num_extensions);
Queue_Family_Indices get_queue_family_indices(Arena *arena, VkPhysicalDevice device,
                                              VkSurfaceKHR surface);

void create_surface(Render_Context *rndr_ctx, GLFWwindow *window_handle);

// Device Stuff //
bool check_device_extension_support(Arena *arena, VkPhysicalDevice device,
                                    const char *const *extensions, u32 num_extensions);
void choose_physical_device(Arena *arena, Render_Context *rndr_ctx);
void create_logical_device(Arena *arena, Render_Context *rndr_ctx);

// Swap Chain Stuff //
Swap_Chain_Info get_swap_chain_info(Arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *formats, u32 num_formats);
VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *modes, u32 num_modes);
VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow *window);
void create_swap_chain(Arena *arena, Render_Context *rndr_ctx, GLFWwindow *window);

// call back for validation layer error messages
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

#endif // RENDER_CONTEXT_H
