#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>

typedef struct Window Window;
struct Window {

    union {
        int width, w;
    };
    union {
        int height, h;
    };

    char *name;
    GLFWwindow *handle;
};

Window window_create(const char *name, int width, int height);
void window_free(Window *window);

bool window_should_close(Window window);

#endif // WINDOW_H
