#ifndef VK_CONTEXT_H
#define VK_CONTEXT_H

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
    VkDevice vk_handle;
    VkQueue graphic_queue;
    VkQueue present_queue;
};

typedef struct Swap_Chain_Info Swap_Chain_Info;
struct Swap_Chain_Info {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    u32 num_formats;
    VkPresentModeKHR *present_modes;
    u32 num_present_modes;
};

typedef struct Render_Context Render_Context;
struct Render_Context {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkPhysicalDevice physical_device;
    Logical_Device logical_device;
    VkSurfaceKHR surface;
};

void render_context_init(Arena *arena, Render_Context *rndr_ctx, GLFWwindow *window_handle);
void render_context_free(Render_Context *rndr_ctx);

// TODO(spencer): Vulkan allows you to specify your own memory allocation function...
// may want to incorporate this with our Arena allocator, or other custom allocator suited to it...
bool check_val_layer_support(Arena *arena, const char **layers, u32 num_layers);
const char **get_glfw_required_extensions(Arena *arena, u32 *num_extensions);

// Device Stuff //
bool check_device_extension_support(Arena *arena, VkPhysicalDevice device, const char **extensions,
                                    u32 num_extensions);
VkPhysicalDevice pick_physical_device(Arena *arena, VkInstance instance);
Logical_Device create_logical_device(Arena *arena, VkPhysicalDevice physical_device,
                                     VkSurfaceKHR surface);

// Swap Chain Stuff //
Swap_Chain_Info get_swap_chain_info(Arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface);
VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *formats, u32 num_formats);
VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *modes, u32 num_modes);
VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR *capabilities, GLFWwindow *window);
VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *window_handle);

// call back for validation layer error messages
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

#endif // VK_CONTEXT_H
