#include "window.h"

#include <stdlib.h>

#include "core/log.h"

translation_local void framebuffer_resize_callback(GLFWwindow *window, int width, int height);

void window_init(Window *window, char *name, u32 width, u32 height) {
  if (!glfwInit()) {
    LOG_FATAL("GLFW failed to initialize", EXT_GLFW_INIT);
  }

  if (!glfwVulkanSupported()) {
    LOG_FATAL("Vulkan is not supported", EXT_VULKAN_SUPPORT);
  }

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  window->handle = glfwCreateWindow(width, height, name, NULL, NULL);
  if (window->handle == NULL) {
    LOG_FATAL("Failed to create GLFW window", EXT_GLFW_WINDOW_CREATION);
  }

  glfwSetWindowUserPointer(window->handle, window);
  glfwSetFramebufferSizeCallback(window->handle, framebuffer_resize_callback);

  if (glfwRawMouseMotionSupported()) {
    LOG_DEBUG("Raw mouse input supported");
    glfwSetInputMode(window->handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(window->handle, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
  }

  window->w = width;
  window->h = height;
  window->resized = false;
  window->name = name;
}

void window_free(Window *window) {
  glfwDestroyWindow(window->handle);
  glfwTerminate();
}

void window_poll_events(void) { glfwPollEvents(); }

bool window_should_close(Window *window) { return glfwWindowShouldClose(window->handle); }

void window_set_to_close(Window *window) { glfwSetWindowShouldClose(window->handle, true); }

// TODO(ss): look into window refresh callback for smoother window resize
translation_local void framebuffer_resize_callback(GLFWwindow *handle, int width, int height) {
  Window *window = (Window *)glfwGetWindowUserPointer(handle);
  window->w = width;
  window->h = height;
  window->resized = true;
}
