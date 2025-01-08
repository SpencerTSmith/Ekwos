#include "window.h"

#include <stdalign.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/common.h"

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);

void window_create(Window *window, const char *name, int width, int height) {
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
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window->handle = glfwCreateWindow(width, height, name, NULL, NULL);
    if (window->handle == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(EXT_GLFW_WINDOW_CREATION);
    }

    glfwSetWindowUserPointer(window->handle, window);
    glfwSetFramebufferSizeCallback(window->handle, framebuffer_resize_callback);

    window->w = width;
    window->h = height;
    window->resized = false;
}

void window_free(Window *window) {
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}

bool window_should_close(Window window) { return glfwWindowShouldClose(window.handle); }

static void framebuffer_resize_callback(GLFWwindow *handle, int width, int height) {
    Window *window = (Window *)glfwGetWindowUserPointer(handle);
    window->w = width;
    window->h = height;
    window->resized = true;
}
