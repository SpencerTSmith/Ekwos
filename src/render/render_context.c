#include "render_context.h"
#include "core/log.h"
#include "render/render_pipeline.h"

#include <stdalign.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
const char *const enabled_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
const u32 num_enable_validation_layers = 1;
const bool enable_val_layers = true;
#else
const char *const enabled_validation_layers[] = NULL;
const u32 num_enable_validation_layers = 0;
const bool enable_val_layers = false;
#endif

const char *const device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
const u32 num_device_extensions = 1;

// Forward declarations //
static void create_instance(Arena *arena, Render_Context *rndr_ctx);

static bool check_val_layer_support(Arena *arena, const char *const *layers, u32 num_layers);
static const char **get_glfw_required_extensions(Arena *arena, u32 *num_extensions);
static Queue_Family_Indices get_queue_family_indices(Arena *arena, VkPhysicalDevice device,
                                                     VkSurfaceKHR surface);

static void create_surface(Render_Context *rndr_ctx, GLFWwindow *window_handle);

// Device Stuff //
static bool check_device_extension_support(Arena *arena, VkPhysicalDevice device,
                                           const char *const *extensions, u32 num_extensions);
static void choose_physical_device(Arena *arena, Render_Context *rndr_ctx);
static void create_logical_device(Arena *arena, Render_Context *rndr_ctx);

// Swap Chain Stuff //
static Swap_Chain_Info get_swap_chain_info(Arena *arena, VkPhysicalDevice device,
                                           VkSurfaceKHR surface);
static VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *formats, u32 num_formats);
static VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *modes, u32 num_modes);
static VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow *window);
static void create_swap_chain(Arena *arena, Render_Context *rndr_ctx, GLFWwindow *window);
static void create_frame_buffers(Render_Context *rndr_ctx);

// Probably will pull this out of being directly associated with Swap
static void create_render_pass(Render_Context *rndr_ctx);

static void create_sync_objects(Render_Context *rndr_ctx);

// Command buffer stuff
void create_command_pool(Render_Context *rndr_ctx);
void alloc_command_buffer(Render_Context *rndr_ctx);

// call back for validation layer error messages
VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data);

void render_context_init(Arena *arena, Render_Context *rndr_ctx, GLFWwindow *window_handle) {
    // Get some temporary memory to hold our queries and such about device supports
    // we don't need it later
    Scratch scratch = scratch_begin(arena);

    create_instance(scratch.arena, rndr_ctx);
    create_surface(rndr_ctx, window_handle);
    choose_physical_device(scratch.arena, rndr_ctx);
    create_logical_device(scratch.arena, rndr_ctx);
    create_swap_chain(scratch.arena, rndr_ctx, window_handle);
    create_render_pass(rndr_ctx);
    create_frame_buffers(rndr_ctx);
    create_command_pool(rndr_ctx);
    alloc_command_buffer(rndr_ctx);
    create_sync_objects(rndr_ctx);

    // Memory storing all those queries not nessecary anymore
    scratch_end(&scratch);

    LOG_DEBUG("Render Context resources initialized");
}

void render_context_free(Render_Context *rndr_ctx) {
    vkDestroySemaphore(rndr_ctx->logical, rndr_ctx->swap.render_finished_sem, NULL);
    vkDestroySemaphore(rndr_ctx->logical, rndr_ctx->swap.image_available_sem, NULL);
    vkDestroyFence(rndr_ctx->logical, rndr_ctx->swap.in_flight_fence, NULL);
    vkDestroyCommandPool(rndr_ctx->logical, rndr_ctx->command_pool, NULL);
    vkDestroyRenderPass(rndr_ctx->logical, rndr_ctx->swap.render_pass, NULL);
    for (u32 i = 0; i < rndr_ctx->swap.image_count; i++) {
        vkDestroyFramebuffer(rndr_ctx->logical, rndr_ctx->swap.framebuffers[i], NULL);
        vkDestroyImageView(rndr_ctx->logical, rndr_ctx->swap.image_views[i], NULL);
    }
    vkDestroySwapchainKHR(rndr_ctx->logical, rndr_ctx->swap.handle, NULL);
    vkDestroySurfaceKHR(rndr_ctx->instance, rndr_ctx->surface, NULL);
    vkDestroyDevice(rndr_ctx->logical, NULL);
    if (enable_val_layers) {
        vkDestroyDebugUtilsMessengerEXT(rndr_ctx->instance, rndr_ctx->debug_messenger, NULL);
    }
    vkDestroyInstance(rndr_ctx->instance, NULL);
    LOG_DEBUG("Render Context resources destroyed");
}

void render_record_command(Render_Context *rc, VkCommandBuffer buf, u32 image_idx,
                           Render_Pipeline *pipeline) {
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // more fields set later?

    VkResult result = vkBeginCommandBuffer(buf, &begin_info);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to begin command buffer recording");
    }
    LOG_DEBUG("Began command buffer recording");

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = rc->swap.render_pass;
    render_pass_info.framebuffer = rc->swap.framebuffers[image_idx];
    render_pass_info.renderArea.offset = (VkOffset2D){0, 0};
    render_pass_info.renderArea.extent = rc->swap.extent;

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};

    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;
    vkCmdBeginRenderPass(buf, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
    LOG_DEBUG("Began render pass");

    vkCmdBindPipeline(rc->command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->handle);
    vkCmdDraw(rc->command_buffer, 3, 1, 0, 0);
    vkCmdEndRenderPass(rc->command_buffer);
    LOG_DEBUG("Ended render_pass");

    result = vkEndCommandBuffer(rc->command_buffer);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to end command buffer recording");
    }
    LOG_DEBUG("Ended command buffer recording");
}

void render_frame(Render_Context *rc, Render_Pipeline *pipeline) {
    // TODO(ss): Should this be wrapped in an assert, deliberate if check? hmmm
    vkWaitForFences(rc->logical, 1, &rc->swap.in_flight_fence, VK_TRUE, UINT64_MAX);
    LOG_DEBUG("Waited for in flight fence");
    vkResetFences(rc->logical, 1, &rc->swap.in_flight_fence);
    LOG_DEBUG("Reset in flight fence");

    // Which framebuffer is ready to be drawn into
    u32 image_idx;
    vkAcquireNextImageKHR(rc->logical, rc->swap.handle, UINT64_MAX, rc->swap.image_available_sem,
                          VK_NULL_HANDLE, &image_idx);
    LOG_DEBUG("Acquired next image from swap chain");
    vkResetCommandBuffer(rc->command_buffer, 0);

    render_record_command(rc, rc->command_buffer, image_idx, pipeline);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &rc->command_buffer;

    // Which semaphores to wait on
    VkSemaphore wait_semaphores[] = {rc->swap.image_available_sem};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;

    // Which semaphores to signal once finished
    VkSemaphore signal_semaphores[] = {rc->swap.render_finished_sem};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    // Give it the fence so we know when it's safe to reuse that command buffer
    VkResult result = vkQueueSubmit(rc->graphic_q, 1, &submit_info, rc->swap.in_flight_fence);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to submit command buffer to graphics queue");
    }

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    // I don't expect we will ever need more than one swap chain
    VkSwapchainKHR swap_chains[] = {rc->swap.handle};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_idx;

    result = vkQueuePresentKHR(rc->present_q, &present_info);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to present image from queue");
    }
}

u32 swap_height(Render_Context *rc) { return rc->swap.extent.height; }
u32 swap_width(Render_Context *rc) { return rc->swap.extent.width; }

void create_instance(Arena *arena, Render_Context *rndr_ctx) {
    // Info needed to create vulkan instance... similar to opengl context
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Ekwos";
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
            LOG_FATAL("Failed to find specified Validation Layers");
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
        LOG_DEBUG("Found adequate layer support");
    }

    // finally, we create insance
    VkResult result = vkCreateInstance(&create_info, NULL, &rndr_ctx->instance);
    if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to create Vulkan Instance");
        exit(EXT_VULKAN_INSTANCE);
    }
    LOG_DEBUG("Created vulkan instance");

    // Messenger needs to be created AFTER vulkan instance creation
    if (enable_val_layers) {
        result = vkCreateDebugUtilsMessengerEXT(rndr_ctx->instance, &debug_create_info, NULL,
                                                &rndr_ctx->debug_messenger);
        if (result != VK_SUCCESS) {
            LOG_FATAL("Failed to create debug messenger");
            exit(EXT_VULKAN_DEBUG_MESSENGER);
        }
        LOG_DEBUG("Created validation messenger");
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
        LOG_FATAL("Failed to get required Vulkan extensions");
        exit(EXT_GLFW_EXTENSIONS);
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
    VkResult result =
        glfwCreateWindowSurface(rndr_ctx->instance, window_handle, NULL, &rndr_ctx->surface);
    if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to create render surface");
        exit(EXT_VULKAN_SURFACE);
    }
    LOG_DEBUG("Created surface");
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
        LOG_FATAL("Failed to find suitable graphics device");
        exit(EXT_VULKAN_DEVICE);
    }
    LOG_DEBUG("Chose physical device");

    rndr_ctx->physical = physical_device;
}

Swap_Chain_Info get_swap_chain_info(Arena *arena, VkPhysicalDevice device, VkSurfaceKHR surface) {
    Swap_Chain_Info info = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &info.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &info.format_count, NULL);
    if (info.format_count == 0) {
        LOG_FATAL("Swap chain support inadequate");
        exit(EXT_VULKAN_SWAP_CHAIN_INFO);
    }

    info.formats = arena_calloc(arena, info.format_count, VkSurfaceFormatKHR);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &info.format_count, info.formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &info.present_mode_count, NULL);
    if (info.format_count == 0) {
        LOG_FATAL("Swap chain support inadequate");
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
            LOG_DEBUG("Found best surface format");
            return formats[i];
        }
    }

    // use the first if we can't find the one we want
    LOG_DEBUG("Failed to find best surface format, falling back to default");
    return formats[0];
}

VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *modes, u32 num_modes) {
    for (u32 i = 0; i < num_modes; i++) {
        // Triple buffering
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            LOG_DEBUG("Chose mailbox present mode");
            return modes[i];
        }
    }

    // If not available, get normal "v-sync"
    LOG_DEBUG("Chose FIFO present mode");
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

    LOG_DEBUG("Surface extent: (%u, %u)", w, h);
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

    // Set to share images if the queues are different... for now
    if (rndr_ctx->graphic_index != rndr_ctx->present_index) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        u32 indices[QUEUE_NUM] = {rndr_ctx->graphic_index, rndr_ctx->present_index};
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
        vkCreateSwapchainKHR(rndr_ctx->logical, &create_info, NULL, &rndr_ctx->swap.handle);
    if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to create swap chain");
        exit(EXT_VULKAN_SWAP_CHAIN_CREATE);
    }
    LOG_DEBUG("Created swap chain");

    vkGetSwapchainImagesKHR(rndr_ctx->logical, rndr_ctx->swap.handle, &rndr_ctx->swap.image_count,
                            NULL);
    LOG_DEBUG("Swap chain using %u images", rndr_ctx->swap.image_count);

    // Grab handles to the swap images
    if (rndr_ctx->swap.image_count > 0 && rndr_ctx->swap.image_count <= MAX_SWAP_IMGS) {
        vkGetSwapchainImagesKHR(rndr_ctx->logical, rndr_ctx->swap.handle,
                                &rndr_ctx->swap.image_count, rndr_ctx->swap.images);
    }

    // Get image views
    for (u32 i = 0; i < rndr_ctx->swap.image_count; i++) {
        VkImageViewCreateInfo iv_info = {0};
        iv_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        iv_info.image = rndr_ctx->swap.images[i];
        iv_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv_info.format = surface_format.format;
        iv_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv_info.subresourceRange.baseMipLevel = 0;
        iv_info.subresourceRange.levelCount = 1;
        iv_info.subresourceRange.baseArrayLayer = 0;
        iv_info.subresourceRange.layerCount = 1;

        result =
            vkCreateImageView(rndr_ctx->logical, &iv_info, NULL, &rndr_ctx->swap.image_views[i]);
        if (result != VK_SUCCESS) {
            LOG_FATAL("Failed to creat swap chain image views");
            exit(EXT_VULKAN_SWAP_CHAIN_IMAGE_VIEW);
        }
        LOG_DEBUG("Created image view [%u]", i);
    }

    rndr_ctx->swap.extent = extent;
    rndr_ctx->swap.format = surface_format.format;
    rndr_ctx->swap.image_count = image_count;
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
        LOG_FATAL("Failed to find suitable queue families");
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
        LOG_FATAL("Failed to create logical device");
        exit(EXT_VULKAN_LOGICAL_DEVICE);
    }
    LOG_DEBUG("Created logical device");

    VkQueue graphic_queue = NULL;
    vkGetDeviceQueue(device, q_fam_indxs.graphic, 0, &graphic_queue);
    LOG_DEBUG("Got graphics device queue");

    VkQueue present_queue = NULL;
    vkGetDeviceQueue(device, q_fam_indxs.present, 0, &present_queue);
    LOG_DEBUG("Got present device queue");

    rndr_ctx->logical = device;
    rndr_ctx->present_q = present_queue;
    rndr_ctx->present_index = q_fam_indxs.present;
    rndr_ctx->graphic_q = graphic_queue;
    rndr_ctx->graphic_index = q_fam_indxs.graphic;
}

void create_frame_buffers(Render_Context *rndr_ctx) {
    Swap_Chain *swap = &rndr_ctx->swap;
    for (u32 i = 0; i < swap->image_count; i++) {
        // Create framebuffers from image views... may want to have more attachments in future...
        // array
        // NOTE(spencer): when changing this in future remember to change attachmentCount in
        // fb_info
        VkImageView attachments[] = {
            swap->image_views[i],
        };

        VkFramebufferCreateInfo fb_info = {0};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.renderPass = swap->render_pass;
        fb_info.attachmentCount = 1;
        fb_info.pAttachments = attachments;
        fb_info.width = swap->extent.width;
        fb_info.height = swap->extent.height;
        fb_info.layers = 1;

        VkResult result =
            vkCreateFramebuffer(rndr_ctx->logical, &fb_info, NULL, &swap->framebuffers[i]);
        if (result != VK_SUCCESS) {
            LOG_FATAL("Failed to create framebuffer %u", i);
            exit(EXT_VULKAN_LOGICAL_DEVICE);
        }
        LOG_DEBUG("Created framebuffer [%u]", i);
    }
}

void create_render_pass(Render_Context *rndr_ctx) {
    VkAttachmentDescription color_attachment = {0};
    color_attachment.format = rndr_ctx->swap.format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;        // maybe more if multisampling?
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear the color attachment
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store the render
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // ready for rendering

    // Can have multiple subpasses that reference other attachments
    VkAttachmentReference color_attachment_ref = {0};
    // this is why we have layout (location = 0) out vec4 outColor in shaders
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {0};
    // We want to wait on the implicit subpass before render pass, basically waiting
    // in render pass
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;

    // What operation to wait on... waiting on swap chain to finish reading image, waiting on color
    // attachment
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;

    // We wait to to do color attachment stage
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VkResult result =
        vkCreateRenderPass(rndr_ctx->logical, &render_pass_info, NULL, &rndr_ctx->swap.render_pass);
    if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to create render pass");
    }
    LOG_DEBUG("Created render pass");
}

void create_command_pool(Render_Context *rc) {
    VkCommandPoolCreateInfo pi = {0};
    pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pi.queueFamilyIndex = rc->graphic_index;

    VkResult result = vkCreateCommandPool(rc->logical, &pi, NULL, &rc->command_pool);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to create command pool");
    }
    LOG_DEBUG("Created command pool");
}

void alloc_command_buffer(Render_Context *rc) {
    VkCommandBufferAllocateInfo cba = {0};
    cba.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cba.commandPool = rc->command_pool;
    cba.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cba.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers(rc->logical, &cba, &rc->command_buffer);
    if (result != VK_SUCCESS) {
        LOG_ERROR("Failed to allocate command buffer");
    }
    LOG_DEBUG("Allocated command buffer");
}

static void create_sync_objects(Render_Context *rndr_ctx) {
    VkSemaphoreCreateInfo sem_info = {0};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkResult result =
        vkCreateSemaphore(rndr_ctx->logical, &sem_info, NULL, &rndr_ctx->swap.image_available_sem);
    if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to create image available semaphore");
        exit(EXT_VULKAN_SYNC_OBJECT);
    }
    LOG_DEBUG("Created image available semaphore");

    result =
        vkCreateSemaphore(rndr_ctx->logical, &sem_info, NULL, &rndr_ctx->swap.render_finished_sem);
    if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to create render finished semaphore");
        exit(EXT_VULKAN_SYNC_OBJECT);
    }
    LOG_DEBUG("Created render finished semaphore");

    result = vkCreateFence(rndr_ctx->logical, &fence_info, NULL, &rndr_ctx->swap.in_flight_fence);
    if (result != VK_SUCCESS) {
        LOG_FATAL("Failed to create in flight fence");
        exit(EXT_VULKAN_SYNC_OBJECT);
    }
    LOG_DEBUG("Created in flight fence");
}

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
    if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOG_FATAL("Validation layer: %s", callback_data->pMessage);
    }
    return VK_FALSE;
}
