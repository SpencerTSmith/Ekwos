#include "window.h"

#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

// What validation layers, may move this out of here
static const char *val_layers[] = {"VK_LAYER_KHRONOS_validation"};
static const u32 num_layers = 1;
static bool enable_val_layers = true;

Window window_create(Arena *arena, const char *name, int width, int height) {
    Window window = {0};

    if (!glfwInit()) {
        fprintf(stderr, "GLFW failed to initialize\n");
        exit(0);
    }

    if (!glfwVulkanSupported()) {
        fprintf(stderr, "Vulkan is not supported\n");
        glfwTerminate();
        exit(0);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window.handle = glfwCreateWindow(width, height, name, NULL, NULL);
    if (window.handle == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXT_GLFW_WINDOW_CREATION);
    }
    window.w = width;
    window.h = height;

    init_vulkan(arena, &window);

    return window;
}

void window_free(Window *window) {
    vkDestroyInstance(window->instance, NULL);
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}

bool window_should_close(Window window) { return glfwWindowShouldClose(window.handle); }

static void init_vulkan(Arena *arena, Window *window) {
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

    // Get some temporary memory to hold our validation layers and extensions
    Scratch scratch = scratch_begin(arena);

    u32 num_extensions = 0;
    const char **extensions = get_required_extensions(scratch, &num_extensions);

    create_info.enabledExtensionCount = num_extensions;
    create_info.ppEnabledExtensionNames = extensions;

    if (enable_val_layers) {
        if (!check_val_layer_support(scratch, val_layers, num_layers)) {
            fprintf(stderr, "Failed to find specified Validation Layers\n");
            exit(EXT_VULKAN_LAYERS);
        }

        create_info.enabledLayerCount = num_layers;
        create_info.ppEnabledLayerNames = val_layers;
    } else {
        create_info.enabledLayerCount = 0;
        create_info.ppEnabledLayerNames = NULL;
    }

    // finally, we create insance
    VkResult result = vkCreateInstance(&create_info, NULL, &window->instance);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan Instance\n");
        // TODO(spencer): Need to create our own shutdown and cleanup function
        glfwTerminate();
        exit(EXT_VULKAN_INSTANCE);
    }

    // Memory storing val layers and extensions no longer needed
    scratch_end(&scratch);
}

static bool check_val_layer_support(Scratch scratch, const char **layers, u32 num_layers) {
    u32 num_supported_layers;
    vkEnumerateInstanceLayerProperties(&num_supported_layers, NULL);

    VkLayerProperties *supported_layers =
        arena_alloc(scratch.arena, num_supported_layers * sizeof(VkLayerProperties),
                    alignof(VkLayerProperties));
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

static const char **get_required_extensions(Scratch scratch, u32 *num_extensions) {
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
        extensions = arena_alloc(scratch.arena, *num_extensions * sizeof(char *), alignof(char *));

        for (u32 i = 0; i < glfw_extension_count; i++) {
            u32 extension_length = strlen(glfw_extensions[i]) + 1;
            char *extension =
                arena_alloc(scratch.arena, extension_length * sizeof(char), alignof(char));

            strcpy(extension, glfw_extensions[i]);
            extensions[i] = extension;
        }

        // Add debug utils layer extension
        u32 debug_layer_extension_length = strlen(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) + 1;
        char *debug_layer_extension =
            arena_alloc(scratch.arena, debug_layer_extension_length * sizeof(char), alignof(char));

        strcpy(debug_layer_extension, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        extensions[*num_extensions - 1] = debug_layer_extension;
    } else {
        *num_extensions = glfw_extension_count;
        extensions = arena_alloc(scratch.arena, *num_extensions * sizeof(char *), alignof(char *));

        for (u32 i = 0; i < glfw_extension_count; i++) {
            u32 extension_length = strlen(glfw_extensions[i]) + 1;
            char *extension =
                arena_alloc(scratch.arena, extension_length * sizeof(char), alignof(char));

            strcpy(extension, glfw_extensions[i]);
            extensions[i] = extension;
        }
    }

    return extensions;
}
