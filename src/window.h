#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct window window;
struct window {

    union {
        int width, w;
    };
    union {
        int height, h;
    };

    char *name;
    GLFWwindow *handle;
};

window create_window(const char *name, int width, int height);
void free_window(window *window);

bool window_should_close(window window);
