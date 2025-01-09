#include "window.h"

#include <stdlib.h>

#include "core/log.h"

static void framebuffer_resize_callback(GLFWwindow *window, int width, int height);

void window_create(Window *window, const char *name, int width, int height) {
    if (!glfwInit()) {
        LOG_FATAL("GLFW failed to initialize");
        exit(EXT_GLFW_INIT);
    }

    if (!glfwVulkanSupported()) {
        LOG_FATAL("Vulkan is not supported");
        glfwTerminate();
        exit(EXT_VULKAN_SUPPORT);
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window->handle = glfwCreateWindow(width, height, name, NULL, NULL);
    if (window->handle == NULL) {
        LOG_FATAL("Failed to create GLFW window");
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

// TODO(ss): look into window refresh callback for smoother window resize
static void framebuffer_resize_callback(GLFWwindow *handle, int width, int height) {
    Window *window = (Window *)glfwGetWindowUserPointer(handle);
    window->w = width;
    window->h = height;
    window->resized = true;
}
