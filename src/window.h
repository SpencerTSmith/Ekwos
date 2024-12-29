#ifndef WINDOW_H
#define WINDOW_H

#include "arena.h"
#include "common.h"
#include "render_context.h"

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

    Render_Context rctx;
};

Window window_create(Arena *arena, const char *name, int width, int height);
void window_free(Window *window);
bool window_should_close(Window window);

#endif // WINDOW_H
