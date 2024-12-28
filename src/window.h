#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>

#include "arena.h"
#include "common.h"

// Contains device and it's queues
typedef struct Logical_Device Logical_Device;
struct Logical_Device {
    VkDevice vk_handle;
    VkQueue graphic_queue;
    VkQueue present_queue;
};

typedef struct Window Window;
struct Window {

    union {
        int width, w;
    };
    union {
        int height, h;
    };

    char *name;
    GLFWwindow *handle;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debug_messenger;

    VkPhysicalDevice physical_device;
    Logical_Device logical_device;

    VkSurfaceKHR surface;
};

typedef struct Swap_Chain_Info Swap_Chain_Info;
struct Swap_Chain_Info {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR *formats;
    u32 num_formats;
    VkPresentModeKHR *present_modes;
    u32 num_present_modes;
};

/*
 *
 * General interface
 *
 */

Window window_create(Arena *arena, const char *name, int width, int height);
void window_free(Window *window);
bool window_should_close(Window window);

/*
 *
 * Vulkan specific functions
 *
 */

// TODO(spencer): Vulkan allows you to specify your own memory allocation function...
// may want to incorporate this with our Arena allocator, or other custom allocator suited to it...
static void init_vulkan(Arena *arena, Window *window);
static bool check_val_layer_support(Arena *arena, const char **layers, u32 num_layers);
static const char **get_glfw_required_extensions(Arena *arena, u32 *num_extensions);

// Device Stuff //
static bool check_device_extension_support(Arena *arena, VkPhysicalDevice device,
                                           const char **extensions, u32 num_extensions);
static VkPhysicalDevice pick_physical_device(Arena *arena, VkInstance instance);
static struct Logical_Device create_logical_device(Arena *arena, VkPhysicalDevice physical_device,
                                                   VkSurfaceKHR surface);

// Swap Chain Stuff //
static Swap_Chain_Info get_swap_chain_info(Arena *arena, VkPhysicalDevice device,
                                           VkSurfaceKHR surface);
static VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *formats, u32 num_formats);
static VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *modes, u32 num_modes);
static VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR *capabilities,
                                     GLFWwindow *window);
static VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *window_handle);

// call back for validation layer error messages
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

#endif // WINDOW_H
