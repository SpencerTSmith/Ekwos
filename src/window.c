#include "window.h"

#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

Window window_create(const char *name, int width, int height) {
    Window window = {0};

    if (!glfwInit()) {
        fprintf(stderr, "GLFW failed to initialize\n");
        exit(EXT_GLFW_INIT);
    }

    if (!glfwVulkanSupported()) {
        fprintf(stderr, "Vulkan is not supported\n");
        glfwTerminate();
        exit(EXT_VULKAN_SUPPORT);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window.handle = glfwCreateWindow(width, height, name, NULL, NULL);
    if (window.handle == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXT_GLFW_WINDOW_CREATION);
    }
    window.w = width;
    window.h = height;

    return window;
}

void window_free(Window *window) {
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}

bool window_should_close(Window window) { return glfwWindowShouldClose(window.handle); }
