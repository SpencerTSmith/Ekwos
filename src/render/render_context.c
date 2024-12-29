#include "render_context.h"

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
    // Get some temporary memory to hold our queries and such about device supports
    // we don't need it later
    Scratch scratch = scratch_begin(arena);

    create_instance(scratch.arena, rndr_ctx);
    create_surface(rndr_ctx, window_handle);
    choose_physical_device(scratch.arena, rndr_ctx);
    create_logical_device(scratch.arena, rndr_ctx);
    create_swap_chain(scratch.arena, rndr_ctx, window_handle);

    // Memory storing all those queries not nessecary anymore
    scratch_end(&scratch);
}

void render_context_free(Render_Context *rndr_ctx) {
    for (u32 i = 0; i < rndr_ctx->swap.image_count; i++) {
        vkDestroyImageView(rndr_ctx->logical.device, rndr_ctx->swap.image_views[i], NULL);
    }
    vkDestroySwapchainKHR(rndr_ctx->logical.device, rndr_ctx->swap.handle, NULL);
    vkDestroySurfaceKHR(rndr_ctx->instance, rndr_ctx->surface, NULL);
    vkDestroyDevice(rndr_ctx->logical.device, NULL);
    if (enable_val_layers) {
        vkDestroyDebugUtilsMessengerEXT(rndr_ctx->instance, rndr_ctx->debug_messenger, NULL);
    }
    vkDestroyInstance(rndr_ctx->instance, NULL);
}

void create_instance(Arena *arena, Render_Context *rndr_ctx) {
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

    u32 num_extensions = 0;
    const char **extensions = get_glfw_required_extensions(arena, &num_extensions);

    create_info.enabledExtensionCount = num_extensions;
    create_info.ppEnabledExtensionNames = extensions;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    if (enable_val_layers) {
        if (!check_val_layer_support(arena, enabled_validation_layers,
                                     num_enable_validation_layers)) {
            fprintf(stderr, "Failed to find specified Validation Layers\n");
            exit(EXT_VULKAN_LAYERS);
        }
        debug_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        // Add back in VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT if needed
        debug_create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
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
}

bool check_val_layer_support(Arena *arena, const char *const *layers, u32 num_layers) {
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

void create_surface(Render_Context *rndr_ctx, GLFWwindow *window_handle) {
    VkSurfaceKHR surface;
    VkResult result = glfwCreateWindowSurface(rndr_ctx->instance, window_handle, NULL, &surface);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create render surface");
        exit(EXT_VULKAN_SURFACE);
    }

    rndr_ctx->surface = surface;
}

bool check_device_extension_support(Arena *arena, VkPhysicalDevice device,
                                    const char *const *extensions, u32 num_extensions) {
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

void choose_physical_device(Arena *arena, Render_Context *rndr_ctx) {
    u32 device_count = 0;
    VkInstance instance = rndr_ctx->instance;

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

    rndr_ctx->physical = physical_device;
}

Swap_Chain_Info get_swap_chain_info(Arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface) {
    Swap_Chain_Info info = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &info.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &info.format_count, NULL);
    if (info.format_count == 0) {
        fprintf(stderr, "Swap chain support inadequate\n");
        exit(EXT_VULKAN_SWAP_CHAIN_INFO);
    }

    info.formats = arena_calloc(arena, info.format_count, VkSurfaceFormatKHR);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &info.format_count, info.formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &info.present_mode_count, NULL);
    if (info.format_count == 0) {
        fprintf(stderr, "Swap chain support inadequate\n");
        exit(EXT_VULKAN_SWAP_CHAIN_INFO);
    }

    info.present_modes = arena_calloc(arena, info.present_mode_count, VkPresentModeKHR);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &info.present_mode_count,
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

VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow *window) {
    // Window manager already specified it for us
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    i32 w, h;
    glfwGetFramebufferSize(window, &w, &h);
    u32 width = w, height = h;
    width = clamp(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    height = clamp(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    VkExtent2D actual_extend = {
        .width = width,
        .height = height,
    };

    return actual_extend;
}

void create_swap_chain(Arena *arena, Render_Context *rndr_ctx, GLFWwindow *window) {
    Swap_Chain_Info info = get_swap_chain_info(arena, rndr_ctx->physical, rndr_ctx->surface);
    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(info.formats, info.format_count);
    VkPresentModeKHR present_mode =
        choose_swap_present_mode(info.present_modes, info.present_mode_count);
    VkExtent2D extent = choose_swap_extent(info.capabilities, window);

    // Suggested to use at least one more
    u32 image_count = info.capabilities.minImageCount + 1;

    // Little clamp
    if (info.capabilities.maxImageCount > 0 && image_count > info.capabilities.maxImageCount) {
        image_count = info.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = rndr_ctx->surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageExtent = extent;
    create_info.imageColorSpace = surface_format.colorSpace;
    // always 1 unless we want to have 3d goggles type thing hahaha
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Queue_Family_Indices q_fam_indxs =
        get_queue_family_indices(arena, rndr_ctx->physical, rndr_ctx->surface);

    // Set to share images if the queues are different... for now
    if (q_fam_indxs.graphic != q_fam_indxs.present) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        u32 indices[QUEUE_NUM] = {q_fam_indxs.graphic, q_fam_indxs.present};
        create_info.queueFamilyIndexCount = QUEUE_NUM;
        create_info.pQueueFamilyIndices = indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // Any transformations we want to apply to images in swapchain
    create_info.preTransform = info.capabilities.currentTransform;

    // Don't make the window transparent
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    // Don't need to keep track of clipped pixels
    create_info.clipped = VK_TRUE;
    // Creating for the first time
    create_info.oldSwapchain = VK_NULL_HANDLE;

    VkResult result =
        vkCreateSwapchainKHR(rndr_ctx->logical.device, &create_info, NULL, &rndr_ctx->swap.handle);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Swap chain support inadequate\n");
        exit(EXT_VULKAN_SWAP_CHAIN_CREATE);
    }

    vkGetSwapchainImagesKHR(rndr_ctx->logical.device, rndr_ctx->swap.handle,
                            &rndr_ctx->swap.image_count, NULL);

    // Grab handles to the swap images
    if (rndr_ctx->swap.image_count > 0 && rndr_ctx->swap.image_count <= MAX_SWAP_IMGS) {
        vkGetSwapchainImagesKHR(rndr_ctx->logical.device, rndr_ctx->swap.handle,
                                &rndr_ctx->swap.image_count, rndr_ctx->swap.images);
    }

    // Get image views
    for (u32 i = 0; i < rndr_ctx->swap.image_count; i++) {
        VkImageViewCreateInfo iv_info = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = rndr_ctx->swap.images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = surface_format.format,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1,
        };

        result = vkCreateImageView(rndr_ctx->logical.device, &iv_info, NULL,
                                   &rndr_ctx->swap.image_views[i]);
        if (result != VK_SUCCESS) {
            fprintf(stderr, "Swap chain support inadequate\n");
            exit(EXT_VULKAN_SWAP_CHAIN_IMAGE_VIEW);
        }
    }

    rndr_ctx->swap.extent = extent;
    rndr_ctx->swap.format = surface_format.format;
}

Queue_Family_Indices get_queue_family_indices(Arena *arena, VkPhysicalDevice device,
                                              VkSurfaceKHR surface) {
    // Find queue families the physical device supports, pick the one that supports graphics and
    // presentation
    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_family_props =
        arena_alloc(arena, queue_family_count * sizeof(VkQueueFamilyProperties),
                    alignof(VkQueueFamilyProperties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_family_props);

    // Get the queue for graphics and presentation
    u32 graphic_index = VK_QUEUE_FAMILY_IGNORED;
    u32 present_index = VK_QUEUE_FAMILY_IGNORED;
    for (u32 i = 0; i < queue_family_count; i++) {
        bool graphic_support = queue_family_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT;
        if (graphic_support) {
            graphic_index = i;
        }

        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support) {
            present_index = i;
        }

        // Found suitable queues for both
        if (graphic_index != VK_QUEUE_FAMILY_IGNORED && present_index != VK_QUEUE_FAMILY_IGNORED) {
            break;
        }
    }

    if (graphic_index == VK_QUEUE_FAMILY_IGNORED || present_index == VK_QUEUE_FAMILY_IGNORED) {
        fprintf(stderr, "Failed to find suitable queue families\n");
        exit(EXT_VULKAN_QUEUE_FAMILIES);
    }

    return (Queue_Family_Indices){.graphic = graphic_index, .present = present_index};
}

void create_logical_device(Arena *arena, Render_Context *rndr_ctx) {
    VkPhysicalDevice physical_device = rndr_ctx->physical;
    VkSurfaceKHR surface = rndr_ctx->surface;

    Queue_Family_Indices q_fam_indxs = get_queue_family_indices(arena, physical_device, surface);

    float queue_priority = 1.0f;

    // Logical device needs an array of queue create infos
    u64 num_queue_creates = 0;
    VkDeviceQueueCreateInfo queue_creates[QUEUE_NUM];

    VkDeviceQueueCreateInfo graphic_create = {0};
    graphic_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphic_create.queueFamilyIndex = q_fam_indxs.graphic;
    graphic_create.queueCount = 1;
    graphic_create.pQueuePriorities = &queue_priority;

    queue_creates[num_queue_creates++] = graphic_create;

    if (q_fam_indxs.graphic != q_fam_indxs.present) {
        VkDeviceQueueCreateInfo present_create = {0};
        present_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        present_create.queueFamilyIndex = q_fam_indxs.present;
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
    vkGetDeviceQueue(device, q_fam_indxs.graphic, 0, &graphic_queue);

    VkQueue present_queue = NULL;
    vkGetDeviceQueue(device, q_fam_indxs.present, 0, &present_queue);

    rndr_ctx->logical = (Logical_Device){
        .device = device,
        .present_q = present_queue,
        .graphic_q = graphic_queue,
    };
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {

    if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "Validation layer: %s\n\n", callback_data->pMessage);
    }
    return VK_FALSE;
}
