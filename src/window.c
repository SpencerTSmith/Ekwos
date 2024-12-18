#include "window.h"

#include <stdio.h>
#include <stdlib.h>

#include "exit.h"

window create_window(const char *name, int width, int height) {
    window window = {};

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

void free_window(window *window) {
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}

bool window_should_close(window window) { return glfwWindowShouldClose(window.handle); }
