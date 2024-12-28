#ifndef WINDOW_H
#define WINDOW_H

#include "arena.h"
#include "common.h"
#include "vk_context.h"

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
    Render_Context rndr_ctx;
};

Window window_create(Arena *arena, const char *name, int width, int height);
void window_free(Window *window);
bool window_should_close(Window window);

#endif // WINDOW_H
