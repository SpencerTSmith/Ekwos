#include "window.h"

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

Window window_create(const char *name, int width, int height) {
    Window window = {};

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window.handle = glfwCreateWindow(width, height, name, NULL, NULL);
    if (window.handle == NULL) {
        fprintf(stderr, "Failed to create GLFW window");
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
