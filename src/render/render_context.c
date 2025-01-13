#include "render_context.h"

#include "core/log.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
static const char *const enabled_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const bool enable_val_layers = true;
#else
static const char *const enabled_validation_layers[] = NULL;
static const u32 num_enable_validation_layers = 0;
static const bool enable_val_layers = false;
#endif // DEBUG defined

static const char *const device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static VkFormat possible_depth_formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                            VK_FORMAT_D24_UNORM_S8_UINT};

// Internals //
typedef struct Swap_Chain_Info Swap_Chain_Info;
struct Swap_Chain_Info {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR surface_formats[RENDER_CONTEXT_MAX_SURFACE_FORMATS];
    u32 surface_format_count;
    VkPresentModeKHR present_modes[RENDER_CONTEXT_MAX_PRESENT_MODES];
    u32 present_mode_count;
};

typedef struct Queue_Family_Indices Queue_Family_Indices;
struct Queue_Family_Indices {
    u32 graphic;
    u32 present;
};

// Forward declarations //
static void create_instance(Arena *arena, RND_Context *rc);
static void choose_physical_device(Arena *arena, RND_Context *rc);
static void create_logical_device(Arena *arena, RND_Context *rc);

// Swap Chain Stuff //
static void create_swap_chain(RND_Context *rc, Window *window);
// Takes in a handle in case we are recreating a swap chain, we can just destroy the old one, as
// specified by the handle
static void destroy_swap_chain(RND_Context *rc, VkSwapchainKHR swap_handle);
static void recreate_swap_chain(RND_Context *rc, Window *window);

void rnd_context_init(Arena *arena, RND_Context *rc, Window *window) {
    // Get some temporary memory to hold our queries and such about device supports
    // we don't need it later
    Scratch scratch = scratch_begin(arena);

    create_instance(scratch.arena, rc);
    rc->surface = window_surface_create(window, rc);
    choose_physical_device(scratch.arena, rc);
    create_logical_device(scratch.arena, rc);
    create_swap_chain(rc, window);

    // Memory storing all those queries not nessecary anymore
    scratch_end(&scratch);

    LOG_DEBUG("Render Context resources initialized");
}

void rnd_context_free(RND_Context *rc) {
    if (rc->instance != VK_NULL_HANDLE) {
        destroy_swap_chain(rc, rc->swap.handle);
        if (rc->surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(rc->instance, rc->surface, NULL);
        } else {
            LOG_ERROR("Tried to destroyed nonexistent vulkan surface");
        }

        if (rc->logical != VK_NULL_HANDLE) {
            vkDestroyDevice(rc->logical, NULL);
        } else {
            LOG_ERROR("Tried to destroyed nonexistent vulkan logical device");
        }

        if (enable_val_layers && rc->debug_messenger != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(rc->instance, rc->debug_messenger, NULL);
        } else {
            LOG_ERROR("Tried to destroyed nonexistent vulkan debug messenger");
        }

        vkDestroyInstance(rc->instance, NULL);
        ZERO_STRUCT(rc);
        LOG_DEBUG("Render Context resources destroyed");
    } else {
        LOG_ERROR("Tried to destroyed nonexistent vulkan instance");
    }
}

// Returns result of acquiring the image, stores the current image index into the
// right field in rc->swap
static VkResult acquire_next_image(RND_Context *rc);

void rnd_begin_frame(RND_Context *rc, Window *window) {
    u32 current_frame = rc->swap.current_frame_idx;

    VkResult result = acquire_next_image(rc);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window->resized) {
        recreate_swap_chain(rc, window);
        window->resized = false;
        LOG_DEBUG("Window resized (%u , %u), swap chain recreated", window->w, window->h);
        // Ok get the next image from the new swap chain, vulkan samples say we can do this
        result = acquire_next_image(rc);
    }

    if (result != VK_SUCCESS) {
        VK_CHECK_FATAL(result, EXT_VK_IMAGE_ACQUIRE, "Unable to acquire next swap chain image");
    }
    LOG_INFO("Acquired next image, %u, from swap chain", rc->swap.current_target_idx);

    VK_CHECK_ERROR(vkResetFences(rc->logical, 1, &rnd_get_current_frame(rc)->in_flight_fence),
                   "Failed to reset in flight fence %u", current_frame);
    LOG_INFO("Waited for and reset in flight fence %u", current_frame);

    VK_CHECK_ERROR(vkResetCommandBuffer(rnd_get_current_cmd(rc), 0),
                   "Failed to reset command buffer %u", current_frame);

    // Ok now ready to start the next frame
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags =
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // we rerecord every frame, check this

    VK_CHECK_ERROR(vkBeginCommandBuffer(rnd_get_current_cmd(rc), &begin_info),
                   "Failed to begin command buffer %u recording", current_frame);
    LOG_INFO("Began command buffer %u recording", current_frame);

    VkOffset2D offset = {0, 0};

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = rc->swap.render_pass;
    render_pass_info.framebuffer = rc->swap.targets[rc->swap.current_target_idx].framebuffer;
    render_pass_info.renderArea.offset = offset;
    render_pass_info.renderArea.extent = rc->swap.extent;

    VkClearValue clear_values[] = {{.color = rc->swap.clear_color},
                                   {.depthStencil = rc->swap.clear_depth}};
    render_pass_info.clearValueCount = STATIC_ARRAY_COUNT(clear_values);
    render_pass_info.pClearValues = clear_values;

    vkCmdBeginRenderPass(rnd_get_current_cmd(rc), &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    // Dynamic State!
    VkViewport viewport = {
        .x = 0.0f,
        .y = 0.0f,
        .width = (f32)rc->swap.extent.width,
        .height = (f32)rc->swap.extent.height,
        .minDepth = 0.0f,
        .maxDepth = 1.0f,
    };
    VkRect2D scissor = {
        .offset = offset,
        .extent = rc->swap.extent,
    };

    vkCmdSetViewport(rnd_get_current_cmd(rc), 0, 1, &viewport);
    vkCmdSetScissor(rnd_get_current_cmd(rc), 0, 1, &scissor);
    LOG_INFO("Began render pass");
}

void rnd_end_frame(RND_Context *rc) {
    u32 current_frame = rc->swap.current_frame_idx;

    vkCmdEndRenderPass(rnd_get_current_cmd(rc));
    LOG_INFO("Ended render_pass");

    VK_CHECK_ERROR(vkEndCommandBuffer(rnd_get_current_cmd(rc)),
                   "Failed to end command buffer %u recording", current_frame);
    LOG_INFO("Ended command buffer %u recording", current_frame);

    VkSemaphore wait_semaphores[] = {rnd_get_current_frame(rc)->image_available_sem};
    VkSemaphore signal_semaphores[] = {rnd_get_current_frame(rc)->render_finished_sem};

    // NOTE(ss): Maybe this one? VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkCommandBuffer cmd = rnd_get_current_cmd(rc);
    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd;
    submit_info.waitSemaphoreCount = STATIC_ARRAY_COUNT(wait_semaphores);
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.signalSemaphoreCount = STATIC_ARRAY_COUNT(signal_semaphores);
    submit_info.pSignalSemaphores = signal_semaphores;

    // Give it the fence so we know when it's safe to reuse that command buffer
    VK_CHECK_ERROR(
        vkQueueSubmit(rc->graphic_q, 1, &submit_info, rnd_get_current_frame(rc)->in_flight_fence),
        "Failed to submit command buffer %u to graphics queue ", current_frame);
    LOG_INFO("Submitted command buffer to graphics queue");

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = STATIC_ARRAY_COUNT(signal_semaphores);
    present_info.pWaitSemaphores = signal_semaphores;
    present_info.swapchainCount = 1; // highly doubt we will ever have more than one
    present_info.pSwapchains = &rc->swap.handle;
    present_info.pImageIndices = &rc->swap.current_target_idx;

    VK_CHECK_ERROR(vkQueuePresentKHR(rc->present_q, &present_info),
                   "Failed to present image from queue");
    LOG_INFO("Queued presentation of image %u", rc->swap.current_target_idx);

    // And increment with wrap around to the next frame resourecs to use
    rc->swap.current_frame_idx = (current_frame + 1) % rc->swap.frames_in_flight;
}

u32 rnd_get_swap_height(const RND_Context *rc) {
    assert(rc != NULL);
    return rc->swap.extent.height;
}
u32 rnd_get_swap_width(const RND_Context *rc) {
    assert(rc != NULL);
    return rc->swap.extent.width;
}

static VkResult acquire_next_image(RND_Context *rc) {
    u32 current_frame = rc->swap.current_frame_idx;

    VK_CHECK_ERROR(vkWaitForFences(rc->logical, 1, &rnd_get_current_frame(rc)->in_flight_fence,
                                   VK_TRUE, UINT64_MAX),
                   "Failed to wait on in flight fence %u", current_frame);

    // Which IMAGE... ie the actual framebuffer is ready to be drawn into
    // This is seperate than per frame resources we keep track of
    VkResult result = vkAcquireNextImageKHR(rc->logical, rc->swap.handle, UINT64_MAX,
                                            rnd_get_current_frame(rc)->image_available_sem,
                                            VK_NULL_HANDLE, &rc->swap.current_target_idx);
    return result;
}

static const char **get_glfw_required_extensions(Arena *arena, u32 *num_extensions) {
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
        extensions = arena_calloc(arena, *num_extensions, const char *);

        for (u32 i = 0; i < glfw_extension_count; i++) {
            u32 extension_length = strlen(glfw_extensions[i]) + 1;
            char *extension = arena_calloc(arena, extension_length, char);

            strcpy(extension, glfw_extensions[i]);
            extensions[i] = extension;
        }

        // Add debug utils layer extension
        u32 debug_layer_extension_length = strlen(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) + 1;
        char *debug_layer_extension = arena_calloc(arena, debug_layer_extension_length, char);

        strcpy(debug_layer_extension, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions[*num_extensions - 1] = debug_layer_extension;
    } else {
        *num_extensions = glfw_extension_count;
        extensions = arena_calloc(arena, *num_extensions, const char *);

        for (u32 i = 0; i < glfw_extension_count; i++) {
            u32 extension_length = strlen(glfw_extensions[i]) + 1;
            char *extension = arena_calloc(arena, extension_length, char);

            strcpy(extension, glfw_extensions[i]);
            extensions[i] = extension;
        }
    }

    return extensions;
}

static bool check_val_layer_support(Arena *arena, const char *const *layers, u32 num_layers) {
    u32 num_supported_layers;
    VK_CHECK_ERROR(vkEnumerateInstanceLayerProperties(&num_supported_layers, NULL),
                   "Failed to enumerate instance layer properties");

    VkLayerProperties *supported_layers =
        arena_calloc(arena, num_supported_layers, VkLayerProperties);
    VK_CHECK_ERROR(vkEnumerateInstanceLayerProperties(&num_supported_layers, supported_layers),
                   "Failed to enumerate instance layer properties");

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

VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagsEXT msg_severity, VkDebugUtilsMessageTypeFlagsEXT msg_type,
    const VkDebugUtilsMessengerCallbackDataEXT *callback_data, void *user_data) {
    if (msg_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        LOG_DEBUG("%s", callback_data->pMessage);
    }
    return VK_FALSE;
}

static void create_instance(Arena *arena, RND_Context *rc) {
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

    VkDebugUtilsMessengerCreateInfoEXT debug_info = {0};
    if (enable_val_layers) {
        if (!check_val_layer_support(arena, enabled_validation_layers,
                                     STATIC_ARRAY_COUNT(enabled_validation_layers))) {
            LOG_FATAL("Failed to find specified Validation Layers");
            exit(EXT_VK_LAYERS);
        }
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debug_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debug_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_info.pfnUserCallback = debug_callback;
        debug_info.pUserData = NULL; // something to add later?

        // Add this extension so we get debug info about creating instances
        create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_info;

        create_info.enabledLayerCount = STATIC_ARRAY_COUNT(enabled_validation_layers);
        create_info.ppEnabledLayerNames = enabled_validation_layers;
        LOG_DEBUG("Found adequate layer support");
    }

    // finally, we create insance
    VK_CHECK_FATAL(vkCreateInstance(&create_info, NULL, &rc->instance), EXT_VK_INSTANCE,
                   "Failed to create Vulkan Instance");
    LOG_DEBUG("Created vulkan instance");

    // Messenger needs to be created AFTER vulkan instance creation
    if (enable_val_layers) {
        VK_CHECK_FATAL(
            vkCreateDebugUtilsMessengerEXT(rc->instance, &debug_info, NULL, &rc->debug_messenger),
            EXT_VK_DEBUG_MESSENGER, "Failed to create debug messenger");
        LOG_DEBUG("Created validation messenger");
    }
}

static bool check_device_extension_support(Arena *arena, VkPhysicalDevice device,
                                           const char *const *extensions, u32 num_extensions) {
    u32 extension_count = 0;

    VK_CHECK_ERROR(vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL),
                   "Failed to enumerate device extension properties");

    VkExtensionProperties *available_extensions =
        arena_calloc(arena, extension_count, VkExtensionProperties);
    VK_CHECK_ERROR(
        vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available_extensions),
        "Failed to enumerate device extension properties");

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

static void choose_physical_device(Arena *arena, RND_Context *rc) {
    u32 device_count = 0;
    VkInstance instance = rc->instance;

    VK_CHECK_ERROR(vkEnumeratePhysicalDevices(instance, &device_count, NULL),
                   "Faield to enumerate physical devices");

    VkPhysicalDevice *phys_devs = arena_calloc(arena, device_count, VkPhysicalDevice);
    VK_CHECK_ERROR(vkEnumeratePhysicalDevices(instance, &device_count, phys_devs),
                   "Faield to enumerate physical devices");

    // Check all devices
    VkPhysicalDevice device_ranking[5] = {0};
    u32 ranking = 0;
    for (u32 i = 0; i < device_count; i++) {
        VkPhysicalDeviceProperties dev_props;
        vkGetPhysicalDeviceProperties(phys_devs[i], &dev_props);
        VkPhysicalDeviceFeatures dev_feats;
        vkGetPhysicalDeviceFeatures(phys_devs[i], &dev_feats);

        // TODO(spencer): rank devices and pick best? for now just pick the Discrete Card that
        // supports a swapchain
        if (check_device_extension_support(arena, phys_devs[i], device_extensions,
                                           STATIC_ARRAY_COUNT(device_extensions))) {
            if (dev_props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                device_ranking[0] = phys_devs[i];
                break;
            } else {
                device_ranking[ranking++] = phys_devs[i];
            }
        }
    }

    if (device_ranking[0] != VK_NULL_HANDLE) {
        rc->physical = device_ranking[0];
        LOG_DEBUG("Chose physical device, discrete GPU with all neede extensions");
        return;
    } else if (ranking > 0) {
        rc->physical = device_ranking[1];
        LOG_DEBUG(
            "Unable to find discrete GPU that supports exension, chose next best physical device");
        return;
    } else {
        LOG_FATAL("Failed to find suitable graphics device");
        exit(EXT_VK_NO_DEVICE);
    }
}

static Queue_Family_Indices get_queue_family_indices(Arena *arena, VkPhysicalDevice device,
                                                     VkSurfaceKHR surface) {
    // Find queue families the physical device supports, pick the one that supports graphics and
    // presentation
    u32 queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties *queue_family_props =
        arena_calloc(arena, queue_family_count, VkQueueFamilyProperties);
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
        VK_CHECK_ERROR(vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support),
                       "Failed  to query physical device surface support");
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
        exit(EXT_VK_QUEUE_FAMILIES);
    }

    return (Queue_Family_Indices){.graphic = graphic_index, .present = present_index};
}

static void create_logical_device(Arena *arena, RND_Context *rc) {
    VkPhysicalDevice physical_device = rc->physical;
    VkSurfaceKHR surface = rc->surface;

    Queue_Family_Indices family_indices = get_queue_family_indices(arena, physical_device, surface);
    rc->graphic_index = family_indices.graphic;
    rc->present_index = family_indices.present;

    f32 queue_priority = 1.0f;

    // Logical device needs an array of queue create infos
    u64 num_queue_creates = 0;
    VkDeviceQueueCreateInfo queue_creates[RENDER_CONTEXT_MAX_QUEUE_NUM];

    VkDeviceQueueCreateInfo graphic_create = {0};
    graphic_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphic_create.queueFamilyIndex = rc->graphic_index;
    graphic_create.queueCount = 1;
    graphic_create.pQueuePriorities = &queue_priority;

    queue_creates[num_queue_creates++] = graphic_create;

    if (rc->graphic_index != rc->present_index) {
        VkDeviceQueueCreateInfo present_create = {0};
        present_create.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        present_create.queueFamilyIndex = rc->present_index;
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
    device_create_info.enabledExtensionCount = STATIC_ARRAY_COUNT(device_extensions);
    device_create_info.ppEnabledExtensionNames = device_extensions;

    if (enable_val_layers) {
        device_create_info.enabledLayerCount = STATIC_ARRAY_COUNT(enabled_validation_layers);
        device_create_info.ppEnabledLayerNames = enabled_validation_layers;
    }

    VK_CHECK_FATAL(vkCreateDevice(physical_device, &device_create_info, NULL, &rc->logical),
                   EXT_VK_LOGICAL_DEVICE, "Failed to create logical device");
    LOG_DEBUG("Created logical device");

    vkGetDeviceQueue(rc->logical, rc->graphic_index, 0, &rc->graphic_q);
    LOG_DEBUG("Got graphics device queue with index %u", rc->graphic_index);

    vkGetDeviceQueue(rc->logical, rc->present_index, 0, &rc->present_q);
    LOG_DEBUG("Got present device queue with index %u", rc->present_index);
}

static Swap_Chain_Info get_swap_chain_info(VkPhysicalDevice device, VkSurfaceKHR surface) {
    Swap_Chain_Info info = {0};
    VK_CHECK_ERROR(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &info.capabilities),
                   "Unable to query device surface capabilities");

    VK_CHECK_ERROR(
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &info.surface_format_count, NULL),
        "Unable to query device surface formats");
    if (info.surface_format_count == 0) {
        LOG_FATAL("Swap chain support inadequate");
        exit(EXT_VK_SWAP_CHAIN_INFO);
    }

    VK_CHECK_ERROR(vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &info.surface_format_count,
                                                        info.surface_formats),
                   "Unable to query device surface formats");

    VK_CHECK_ERROR(
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &info.present_mode_count, NULL),
        "Unable to query device surface present modes");
    if (info.present_mode_count == 0) {
        LOG_FATAL("Swap chain support inadequate");
        exit(EXT_VK_SWAP_CHAIN_INFO);
    }

    VK_CHECK_ERROR(vkGetPhysicalDeviceSurfacePresentModesKHR(
                       device, surface, &info.present_mode_count, info.present_modes),
                   "Unable to query device surface present modes");

    return info;
}

static VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *formats, u32 num_formats) {
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

static VkFormat choose_swap_depth_format(VkPhysicalDevice device, VkFormat *formats,
                                         u32 num_formats, VkImageTiling tiling,
                                         VkFormatFeatureFlags features) {
    for (u32 i = 0; i < num_formats; i++) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(device, formats[i], &props);
        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return formats[i];
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return formats[i];
        }
    }

    LOG_FATAL("Failed to find supported depth format");
    exit(EXT_VK_DEPTH_FORMAT);
}

static VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *modes, u32 num_modes) {
    for (u32 i = 0; i < num_modes; i++) {
        // Triple buffering
        if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
            LOG_DEBUG("Chose mailbox present mode");
            return modes[i];
        }
    }

    // If not available, get normal "v-sync", it must be supported by all implementations
    LOG_DEBUG("Chose FIFO present mode");
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR capabilities, Window *window) {
    // Window manager already specified it for us
    if (capabilities.currentExtent.width != UINT32_MAX) {
        return capabilities.currentExtent;
    }

    u32 w = window->w, h = window->h;
    w = CLAMP(w, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    h = CLAMP(h, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    VkExtent2D actual_extend = {
        .width = w,
        .height = h,
    };

    LOG_DEBUG("Surface extent: (%u, %u)", w, h);
    return actual_extend;
}

static void create_render_pass(RND_Context *rc);
static void create_target_resources(RND_Context *rc);
static void create_frame_resources(RND_Context *rc);

static void create_swap_chain(RND_Context *rc, Window *window) {
    Swap_Chain_Info info = get_swap_chain_info(rc->physical, rc->surface);

    // Get all that info in there
    rc->swap.surface_format =
        choose_swap_surface_format(info.surface_formats, info.surface_format_count);
    rc->swap.present_mode = choose_swap_present_mode(info.present_modes, info.present_mode_count);
    rc->swap.extent = choose_swap_extent(info.capabilities, window);
    rc->swap.depth_format = choose_swap_depth_format(
        rc->physical, possible_depth_formats, STATIC_ARRAY_COUNT(possible_depth_formats),
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    rc->swap.clear_color = (VkClearColorValue){.float32 = {0.0f, 0.f, 0.0f, 1.0f}};
    rc->swap.clear_depth = (VkClearDepthStencilValue){.depth = 1.0f, .stencil = 0};

    // Suggested to use at least one more, if possible
    rc->swap.target_count =
        MIN(info.capabilities.minImageCount + 1, RENDER_CONTEXT_MAX_SWAP_IMAGES);
    if (info.capabilities.maxImageCount > 0) {
        rc->swap.target_count =
            MIN(RENDER_CONTEXT_MAX_SWAP_IMAGES, info.capabilities.maxImageCount);
    }

    // If we are recreating
    VkSwapchainKHR old_swapchain = rc->swap.handle != NULL ? rc->swap.handle : VK_NULL_HANDLE;

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = rc->surface;
    create_info.minImageCount = rc->swap.target_count;
    create_info.imageFormat = rc->swap.surface_format.format;
    create_info.imageExtent = rc->swap.extent;
    create_info.imageColorSpace = rc->swap.surface_format.colorSpace;
    create_info.presentMode = rc->swap.present_mode;
    // always 1 unless we want to have 3d goggles type thing hahaha
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Set to share images if the queues are different... for now
    if (rc->graphic_index != rc->present_index) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        u32 indices[RENDER_CONTEXT_MAX_QUEUE_NUM] = {rc->graphic_index, rc->present_index};
        create_info.queueFamilyIndexCount = RENDER_CONTEXT_MAX_QUEUE_NUM;
        create_info.pQueueFamilyIndices = indices;
    } else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // Any transformations we want to apply to images in swapchain
    create_info.preTransform = info.capabilities.currentTransform;

    // Don't make the window transparent
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Don't need to keep track of clipped pixels
    create_info.clipped = VK_TRUE;
    // If we are recreating, pass in the old one
    create_info.oldSwapchain = old_swapchain;

    VK_CHECK_FATAL(vkCreateSwapchainKHR(rc->logical, &create_info, NULL, &rc->swap.handle),
                   EXT_VK_SWAP_CHAIN_CREATE, "Failed to create swapchain");
    LOG_DEBUG("Created swap chain");

    if (old_swapchain != VK_NULL_HANDLE) {
        destroy_swap_chain(rc, old_swapchain);
    }

    // TODO(ss): check if we need to recreate the render pass, we may not need to
    rc->swap.arena = rnd_allocator_init(rc, 1024);
    create_render_pass(rc);
    create_target_resources(rc);
    create_frame_resources(rc);
}

// TODO(ss): solve this, make it so it just calls the original recreate swap chain
static void recreate_swap_chain(RND_Context *rc, Window *window) {
    VkExtent2D extent = {window->w, window->h};

    // Wait while either dimension is 0, and until device is idle
    while (extent.width == 0 || extent.height == 0) {
        extent = (VkExtent2D){window->w, window->h};
        // And this will wait until another widnow resize callback
        glfwWaitEvents();
    }

    VK_CHECK_ERROR(vkDeviceWaitIdle(rc->logical),
                   "Failed to wait for device idle in recreation of swap_chain");

    create_swap_chain(rc, window);
}

static void create_render_pass(RND_Context *rc) {
    VkAttachmentDescription color_attachment = {0};
    color_attachment.format = rc->swap.surface_format.format;
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

    VkAttachmentDescription depth_attachment = {0};
    depth_attachment.format = rc->swap.depth_format;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref = {0};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency = {0};
    dependency.dstSubpass = 0;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    VkAttachmentDescription attachments[] = {
        color_attachment,
        depth_attachment,
    };
    VkRenderPassCreateInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = STATIC_ARRAY_COUNT(attachments);
    render_pass_info.pAttachments = attachments;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VK_CHECK_FATAL(vkCreateRenderPass(rc->logical, &render_pass_info, NULL, &rc->swap.render_pass),
                   EXT_VK_RENDER_PASS_CREATE, "Failed to create render pass");
    LOG_DEBUG("Created render pass");
}

static void create_target_resources(RND_Context *rc) {
    VK_CHECK_ERROR(
        vkGetSwapchainImagesKHR(rc->logical, rc->swap.handle, &rc->swap.target_count, NULL),
        "Unable to get swap chain images");
    LOG_DEBUG("Swap chain using %u images", rc->swap.target_count);

    // Grab handles to the swap images
    VkImage temp_image_array[RENDER_CONTEXT_MAX_SWAP_IMAGES] = {0};
    if (rc->swap.target_count > 0 && rc->swap.target_count <= RENDER_CONTEXT_MAX_SWAP_IMAGES) {
        vkGetSwapchainImagesKHR(rc->logical, rc->swap.handle, &rc->swap.target_count,
                                temp_image_array);
    }

    // Get image views
    for (u32 i = 0; i < rc->swap.target_count; i++) {
        // Color Resources
        rc->swap.targets[i].color_image = temp_image_array[i];

        VkImageViewCreateInfo iv_info = {0};
        iv_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        iv_info.image = rc->swap.targets[i].color_image;
        iv_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv_info.format = rc->swap.surface_format.format;
        iv_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        iv_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv_info.subresourceRange.baseMipLevel = 0;
        iv_info.subresourceRange.levelCount = 1;
        iv_info.subresourceRange.baseArrayLayer = 0;
        iv_info.subresourceRange.layerCount = 1;

        VK_CHECK_FATAL(
            vkCreateImageView(rc->logical, &iv_info, NULL, &rc->swap.targets[i].color_image_view),
            EXT_VK_SWAP_CHAIN_IMAGE_VIEW, "Failed to creat swap chain image view %u", i);
        LOG_DEBUG("Created swap chain image view %u", i);

        // Depth Resources
        VkImageCreateInfo depth_image_info = {0}; // Image
        depth_image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depth_image_info.imageType = VK_IMAGE_TYPE_2D;
        depth_image_info.extent.width = rc->swap.extent.width;
        depth_image_info.extent.height = rc->swap.extent.height;
        depth_image_info.extent.depth = 1;
        depth_image_info.mipLevels = 1;
        depth_image_info.arrayLayers = 1;
        depth_image_info.format = rc->swap.depth_format;
        depth_image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        depth_image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        depth_image_info.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        depth_image_info.flags = 0;

        rnd_alloc_image(&rc->swap.arena, rc, depth_image_info, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        &rc->swap.targets[i].depth_image, &rc->swap.targets[i].depth_memory);
        LOG_DEBUG("Allocated memory for swap chain depth image %u", i);

        VkImageViewCreateInfo depth_view_info = {0};
        depth_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depth_view_info.image = rc->swap.targets[i].depth_image;
        depth_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depth_view_info.format = rc->swap.depth_format;
        depth_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depth_view_info.subresourceRange.baseMipLevel = 0;
        depth_view_info.subresourceRange.levelCount = 1;
        depth_view_info.subresourceRange.baseArrayLayer = 0;
        depth_view_info.subresourceRange.layerCount = 1;

        VK_CHECK_FATAL(vkCreateImageView(rc->logical, &depth_view_info, NULL,
                                         &rc->swap.targets[i].depth_image_view),
                       EXT_VK_DEPTH_VIEW, "Failed to create swap chain depth image veiw");
        LOG_DEBUG("Created swap chain depth image view %u", i);
    }

    // Create framebuffers from image views... may want to have more attachments in future...
    // array
    for (u32 i = 0; i < rc->swap.target_count; i++) {
        VkImageView attachments[] = {
            rc->swap.targets[i].color_image_view,
            rc->swap.targets[i].depth_image_view,
        };

        VkFramebufferCreateInfo fb_info = {0};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.renderPass = rc->swap.render_pass;
        fb_info.attachmentCount = STATIC_ARRAY_COUNT(attachments);
        fb_info.pAttachments = attachments;
        fb_info.width = rc->swap.extent.width;
        fb_info.height = rc->swap.extent.height;
        fb_info.layers = 1;

        VK_CHECK_FATAL(
            vkCreateFramebuffer(rc->logical, &fb_info, NULL, &rc->swap.targets[i].framebuffer),
            EXT_VK_LOGICAL_DEVICE, "Failed to create framebuffer %u", i);
        LOG_DEBUG("Created framebuffer %u", i);
    }
}

static void create_frame_resources(RND_Context *rc) {
    VkCommandPoolCreateInfo pi = {0};
    pi.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pi.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    // NOTE(ss): research this
    pi.queueFamilyIndex = rc->graphic_index;

    VK_CHECK_FATAL(vkCreateCommandPool(rc->logical, &pi, NULL, &rc->swap.command_pool),
                   EXT_VK_COMMAND_POOL, "Failed to create command pool");
    LOG_DEBUG("Created command pool");

    // TODO(ss): some time of configuration or checks on this number
    rc->swap.frames_in_flight = RENDER_CONTEXT_MAX_FRAMES_IN_FLIGHT;

    VkCommandBufferAllocateInfo cba = {0};
    cba.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cba.commandPool = rc->swap.command_pool;
    cba.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cba.commandBufferCount = rc->swap.frames_in_flight;

    VkCommandBuffer temp_command_buffer_array[RENDER_CONTEXT_MAX_FRAMES_IN_FLIGHT];
    VK_CHECK_FATAL(vkAllocateCommandBuffers(rc->logical, &cba, temp_command_buffer_array),
                   EXT_VK_COMMAND_BUFFER, "Failed to allocate command buffer");
    for (u32 i = 0; i < rc->swap.frames_in_flight; i++) {
        rc->swap.frames[i].command_buffer = temp_command_buffer_array[i];
    }

    LOG_DEBUG("Allocated command buffers");

    VkSemaphoreCreateInfo sem_info = {0};
    sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (u32 i = 0; i < rc->swap.frames_in_flight; i++) {
        VK_CHECK_FATAL(vkCreateSemaphore(rc->logical, &sem_info, NULL,
                                         &rc->swap.frames[i].image_available_sem),
                       EXT_VK_SYNC_OBJECT, "Failed to create image available semaphore");
        LOG_DEBUG("Created image available semaphore %u", i);

        VK_CHECK_FATAL(vkCreateSemaphore(rc->logical, &sem_info, NULL,
                                         &rc->swap.frames[i].render_finished_sem),
                       EXT_VK_SYNC_OBJECT, "Failed to create render_finished semaphore");
        LOG_DEBUG("Created render finished semaphore %u", i);

        VK_CHECK_FATAL(
            vkCreateFence(rc->logical, &fence_info, NULL, &rc->swap.frames[i].in_flight_fence),
            EXT_VK_SYNC_OBJECT, "Failed to create in flight fence");
        LOG_DEBUG("Created in flight fence %u", i);
    }
}

static void destroy_swap_chain(RND_Context *rc, VkSwapchainKHR swap_handle) {
    if (swap_handle != VK_NULL_HANDLE && rc->logical != VK_NULL_HANDLE) {
        for (u32 i = 0; i < rc->swap.frames_in_flight; i++) {
            if (rc->swap.frames[i].render_finished_sem != VK_NULL_HANDLE) {
                vkDestroySemaphore(rc->logical, rc->swap.frames[i].render_finished_sem, NULL);
            } else {
                LOG_ERROR("Tried to destroy nonexistent vulkan render finished semaphore");
            }

            if (rc->swap.frames[i].image_available_sem != VK_NULL_HANDLE) {
                vkDestroySemaphore(rc->logical, rc->swap.frames[i].image_available_sem, NULL);
            } else {
                LOG_ERROR("Tried to destroy nonexistent vulkan image available semaphore");
            }

            if (rc->swap.frames[i].in_flight_fence != VK_NULL_HANDLE) {
                vkDestroyFence(rc->logical, rc->swap.frames[i].in_flight_fence, NULL);
            } else {
                LOG_ERROR("Tried to destroy nonexistent vulkan image available semaphore");
            }
        }

        if (rc->swap.command_pool != VK_NULL_HANDLE) {
            vkDestroyCommandPool(rc->logical, rc->swap.command_pool, NULL);
        } else {
            LOG_ERROR("Tried to destroy nonexistent vulkan command pool");
        }

        if (rc->swap.render_pass != VK_NULL_HANDLE) {
            vkDestroyRenderPass(rc->logical, rc->swap.render_pass, NULL);
        } else {
            LOG_ERROR("Tried to destroy nonexistent vulkan render pass");
        }

        for (u32 i = 0; i < rc->swap.target_count; i++) {
            if (rc->swap.targets[i].framebuffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(rc->logical, rc->swap.targets[i].framebuffer, NULL);
            } else {
                LOG_ERROR("Tried to destroy nonexistent vulkan framebuffer");
            }

            if (rc->swap.targets[i].color_image_view != VK_NULL_HANDLE) {
                vkDestroyImageView(rc->logical, rc->swap.targets[i].color_image_view, NULL);
            } else {
                LOG_ERROR("Tried to destroy nonexistent vulkan image view");
            }

            if (rc->swap.targets[i].depth_image_view != VK_NULL_HANDLE) {
                vkDestroyImage(rc->logical, rc->swap.targets[i].depth_image, NULL);
                vkDestroyImageView(rc->logical, rc->swap.targets[i].depth_image_view, NULL);
                vkFreeMemory(rc->logical, rc->swap.targets[i].depth_memory, NULL);
            } else {
                LOG_ERROR("Tried to destroy nonexistent vulkan depth image view");
            }
        }

        vkDestroySwapchainKHR(rc->logical, swap_handle, NULL);
        // ZERO_STRUCT(&rc->swap);
        LOG_DEBUG("Destroyed swap chain resources");
    } else {
        LOG_DEBUG("Tried to destroy nonexistent vulkan swap chain");
    }
}
