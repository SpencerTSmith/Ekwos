#include "vk_context.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *const enabled_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
const u32 num_enable_validation_layers = 1;
const bool enable_val_layers = true;

const char *const device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const u32 num_device_extensions = 1;

void render_context_init(Arena *arena, Render_Context *rndr_ctx, GLFWwindow *window_handle) {
    // Info needed to create vulkan instance... similar to opengl context
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Game";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    // Any extra info needed
    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    // Get some temporary memory to hold our validation layers and extensions,
    // we don't need it later
    Scratch scratch = scratch_begin(arena);

    u32 num_extensions = 0;
    const char **extensions = get_glfw_required_extensions(scratch.arena, &num_extensions);

    create_info.enabledExtensionCount = num_extensions;
    create_info.ppEnabledExtensionNames = extensions;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    if (enable_val_layers) {
        if (!check_val_layer_support(scratch.arena, enabled_validation_layers,
                                     num_enable_validation_layers)) {
            fprintf(stderr, "Failed to find specified Validation Layers\n");
            exit(EXT_VULKAN_LAYERS);
        }
        debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_create_info.pfnUserCallback = debug_callback;
        debug_create_info.pUserData = NULL; // something to add later?

        // Add this extension so we get debug info about creating instances
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;

        create_info.enabledLayerCount = num_enable_validation_layers;
        create_info.ppEnabledLayerNames = enabled_validation_layers;
    }

    // finally, we create insance
    VkResult result = vkCreateInstance(&create_info, NULL, &rndr_ctx->instance);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan Instance\n");
        // TODO(spencer): Create own shutdown and cleanup function?
        exit(EXT_VULKAN_INSTANCE);
    }

    // Messenger needs to be created AFTER vulkan instance creation
    if (enable_val_layers) {
        result = vkCreateDebugUtilsMessengerEXT(rndr_ctx->instance, &debug_create_info, NULL,
                                                &rndr_ctx->debug_messenger);
        if (result != VK_SUCCESS) {
            fprintf(stderr, "Failed to create debug messenger\n");
            exit(EXT_VULKAN_DEBUG_MESSENGER);
        }
    }

    // All other things for our vulkan initialization
    rndr_ctx->surface = create_surface(rndr_ctx->instance, window_handle);
    rndr_ctx->physical_device = pick_physical_device(scratch.arena, rndr_ctx->instance);
    rndr_ctx->logical_device =
        create_logical_device(scratch.arena, rndr_ctx->physical_device, rndr_ctx->surface);

    // Memory storing all this no longer nessecary
    scratch_end(&scratch);
}

void render_context_free(Render_Context *rndr_ctx) {
    vkDestroySurfaceKHR(rndr_ctx->instance, rndr_ctx->surface, NULL);
    vkDestroyDevice(rndr_ctx->logical_device.vk_handle, NULL);
    if (enable_val_layers) {
        vkDestroyDebugUtilsMessengerEXT(rndr_ctx->instance, rndr_ctx->debug_messenger, NULL);
    }
    vkDestroyInstance(rndr_ctx->instance, NULL);
}

bool check_val_layer_support(Arena *arena, const char **layers, u32 num_layers) {
    u32 num_supported_layers;
    vkEnumerateInstanceLayerProperties(&num_supported_layers, NULL);

    VkLayerProperties *supported_layers = arena_alloc(
        arena, num_supported_layers * sizeof(VkLayerProperties), alignof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&num_supported_layers, supported_layers);

    for (u32 i = 0; i < num_layers; i++) {
        bool found = false;

        for (u32 j = 0; j < num_supported_layers; j++) {
            if (strcmp(layers[i], supported_layers[j].layerName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

const char **get_glfw_required_extensions(Arena *arena, u32 *num_extensions) {
    u32 glfw_extension_count = 0;
    const char **glfw_extensions;
    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    if (!glfw_extensions) {
        fprintf(stderr, "Failed to get required Vulkan extensions\n");
        glfwTerminate();
        exit(0);
    }

    const char **extensions = NULL;
    if (enable_val_layers) {
        *num_extensions = glfw_extension_count + 1;
        extensions = arena_alloc(arena, *num_extensions * sizeof(char *), alignof(char *));

        for (u32 i = 0; i < glfw_extension_count; i++) {
            u32 extension_length = strlen(glfw_extensions[i]) + 1;
            char *extension = arena_alloc(arena, extension_length * sizeof(char), alignof(char));

            strcpy(extension, glfw_extensions[i]);
            extensions[i] = extension;
        }

        // Add debug utils layer extension
        u32 debug_layer_extension_length = strlen(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) + 1;
        char *debug_layer_extension =
            arena_alloc(arena, debug_layer_extension_length * sizeof(char), alignof(char));

        strcpy(debug_layer_extension, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions[*num_extensions - 1] = debug_layer_extension;
    } else {
        *num_extensions = glfw_extension_count;
        extensions = arena_alloc(arena, *num_extensions * sizeof(char *), alignof(char *));

        for (u32 i = 0; i < glfw_extension_count; i++) {
            u32 extension_length = strlen(glfw_extensions[i]) + 1;
            char *extension = arena_alloc(arena, extension_length * sizeof(char), alignof(char));

            strcpy(extension, glfw_extensions[i]);
            extensions[i] = extension;
        }
    }

    return extensions;
}

bool check_device_extension_support(Arena *arena, VkPhysicalDevice device, const char **extensions,
                                    u32 num_extensions) {
    u32 extension_count = 0;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

    VkExtensionProperties *available_extensions =
        arena_calloc(arena, extension_count, VkExtensionProperties);
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available_extensions);

    for (u32 i = 0; i < num_extensions; i++) {
        bool found = false;

        for (u32 j = 0; j < extension_count; j++) {
            if (strcmp(extensions[i], available_extensions[j].extensionName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            return false;
        }
    }

    return true;
}

VkPhysicalDevice pick_physical_device(Arena *arena, VkInstance instance) {
    u32 device_count = 0;
    vkEnumeratePhysicalDevices(instance, &device_count, NULL);

    VkPhysicalDevice *phys_devs =
        arena_alloc(arena, device_count * sizeof(VkPhysicalDevice), alignof(VkPhysicalDevice));
    vkEnumeratePhysicalDevices(instance, &device_count, phys_devs);

    // Check all devices
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;
    for (u32 i = 0; i < device_count; i++) {
        VkPhysicalDeviceProperties dev_props;
        vkGetPhysicalDeviceProperties(phys_devs[i], &dev_props);
        VkPhysicalDeviceFeatures dev_feats;
        vkGetPhysicalDeviceFeatures(phys_devs[i], &dev_feats);

        // TODO(spencer): rank devices and pick best? for now just pick the Discrete Card that
        // supports a swapchain
        if (dev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
            check_device_extension_support(arena, phys_devs[i], device_extensions,
                                           num_device_extensions)) {
            physical_device = phys_devs[i];
            break;
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        fprintf(stderr, "Failed to find suitable graphics device\n");
        exit(EXT_VULKAN_DEVICE);
    }
    return physical_device;
}

Swap_Chain_Info get_swap_chain_info(Arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface) {
    Swap_Chain_Info info = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &info.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &info.num_formats, NULL);
    if (info.num_formats == 0) {
        fprintf(stderr, "Swap chain support inadequate\n");
        exit(EXT_VULKAN_SWAP_CHAIN_SUPPORT);
    }

    info.formats = arena_calloc(arena, info.num_formats, VkSurfaceFormatKHR);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &info.num_formats, info.formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &info.num_present_modes, NULL);
    if (info.num_formats == 0) {
        fprintf(stderr, "Swap chain support inadequate\n");
        exit(EXT_VULKAN_SWAP_CHAIN_SUPPORT);
    }

    info.present_modes = arena_calloc(arena, info.num_present_modes, VkPresentModeKHR);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &info.num_present_modes,
                                              info.present_modes);

    return info;
}

VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *formats, u32 num_formats) {
    for (u32 i = 0; i < num_formats; i++) {
        if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return formats[i];
        }
    }

    // use the first if we can't find the one we want
    return formats[0];
}

VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *modes, u32 num_modes) {
    for (u32 i = 0; i < num_modes; i++) {
        // Triple buffering
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            return modes[i];
        }
    }

    // If not available, get normal "v-sync"
    return VK_PRESENT_MODE_FIFO_KHR;
}

#define clamp(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR *capabilities, GLFWwindow *window) {
    // Window manager already specified it for us
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }

    i32 w, h;
    glfwGetFramebufferSize(window, &w, &h);
    u32 width = w, height = h;
    width = clamp(width, capabilities->minImageExtent.width, capabilities->maxImageExtent.width);
    height =
        clamp(height, capabilities->minImageExtent.height, capabilities->maxImageExtent.height);

    VkExtent2D actual_extend = {
        .width = width,
        .height = height,
    };

    return actual_extend;
}

struct Logical_Device create_logical_device(Arena *arena, VkPhysicalDevice physical_device,
                                            VkSurfaceKHR surface) {
    // Find queue families the physical device supports, pick the one that supports graphics and
    // presentation
    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_family_props =
        arena_alloc(arena, queue_family_count * sizeof(VkQueueFamilyProperties),
                    alignof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count,
                                             queue_family_props);

    // Get the queue for graphics and presentation
    i64 graphic_index = -1;
    i64 present_index = -1;
    for (u32 i = 0; i < queue_family_count; i++) {
        bool graphic_support = queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
        if (graphic_support) {
            graphic_index = i;
        }

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
        if (present_support) {
            present_index = i;
        }

        // Found suitable queues for both
        if (graphic_index != -1 && present_index != -1) {
            break;
        }
    }

    if (graphic_index == -1 || present_index == -1) {
        fprintf(stderr, "Failed to find suitable queue families\n");
        exit(EXT_VULKAN_QUEUE_FAMILIES);
    }

    float queue_priority = 1.0f;

    // Logical device needs an array of queue create infos
    u64 num_queue_creates = 0;
    VkDeviceQueueCreateInfo queue_creates[2];

    VkDeviceQueueCreateInfo graphic_create = {0};
    graphic_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphic_create.queueFamilyIndex = graphic_index;
    graphic_create.queueCount = 1;
    graphic_create.pQueuePriorities = &queue_priority;

    queue_creates[num_queue_creates++] = graphic_create;

    if (graphic_index != present_index) {
        VkDeviceQueueCreateInfo present_create = {0};
        present_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        present_create.queueFamilyIndex = present_index;
        present_create.queueCount = 1;
        present_create.pQueuePriorities = &queue_priority;

        queue_creates[num_queue_creates++] = present_create;
    }

    // NOTE(spencer): probably need stuff here... later
    VkPhysicalDeviceFeatures device_features = {0};

    VkDeviceCreateInfo device_create_info = {0};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = num_queue_creates;
    device_create_info.pQueueCreateInfos = queue_creates;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = num_device_extensions;
    device_create_info.ppEnabledExtensionNames = device_extensions;

    if (enable_val_layers) {
        device_create_info.enabledLayerCount = num_enable_validation_layers;
        device_create_info.ppEnabledLayerNames = enabled_validation_layers;
    }

    VkDevice device = NULL;
    VkResult result = vkCreateDevice(physical_device, &device_create_info, NULL, &device);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device");
        exit(EXT_VULKAN_LOGICAL_DEVICE);
    }

    VkQueue graphic_queue = NULL;
    vkGetDeviceQueue(device, graphic_index, 0, &graphic_queue);

    VkQueue present_queue = NULL;
    vkGetDeviceQueue(device, present_index, 0, &present_queue);

    return (struct Logical_Device){
        .vk_handle = device,
        .graphic_queue = graphic_queue,
        .present_queue = present_queue,
    };
}

VkSurfaceKHR create_surface(VkInstance instance, GLFWwindow *window_handle) {
    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface(instance, window_handle, NULL, &surface);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create render surface");
        exit(EXT_VULKAN_SURFACE);
    }

    return surface;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {

    if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "\n\nValidation layer: %s\n\n", callback_data->pMessage);
    }
    return VK_FALSE;
}
