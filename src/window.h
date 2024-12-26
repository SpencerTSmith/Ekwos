#ifndef WINDOW_H
#define WINDOW_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdbool.h>

#include "arena.h"
#include "common.h"

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
    VkInstance instance;
};

Window window_create(Arena *arena, const char *name, int width, int height);
void window_free(Window *window);
bool window_should_close(Window window);

// TODO(spencer): Vulkan allow you to specify your own memory allocation function...
// may want to incorporate this with our Arena allocator, or other custom allocator suited to it...
static void init_vulkan(Arena *arena, Window *window);
static bool check_val_layer_support(Scratch scratch, const char **layers, u32 num_layers);
static const char **get_required_extensions(Scratch scratch, u32 *num_extensions);

#endif // WINDOW_H
