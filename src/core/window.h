#ifndef WINDOW_H
#define WINDOW_H

#include "core/common.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdbool.h>

typedef struct Window Window;
struct Window {
    union {
        u32 width, w;
    };
    union {
        u32 height, h;
    };
    GLFWwindow *handle;
    char *name;
    bool resized;
    f64 cursor_x, cursor_y;
};

void window_init(Window *window, const char *name, int width, int height);
void window_free(Window *window);

void poll_events(void);
bool window_should_close(Window *window);
void window_set_to_close(Window *window);

typedef struct RND_Context RND_Context;
VkSurfaceKHR window_surface_create(Window *window, RND_Context *rc);

#endif // WINDOW_H
