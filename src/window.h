#ifndef WINDOW_H
#define WINDOW_H

#include "core/common.h"
#include "render/render_context.h"

#include <stdbool.h>

typedef struct Window Window;
struct Window {
    union {
        u32 width, w;
    };
    union {
        u32 height, h;
    };
    char *name;
    GLFWwindow *handle;

    Render_Context rctx;
};

Window window_create(const char *name, int width, int height);
void window_free(Window *window);
bool window_should_close(Window window);

#endif // WINDOW_H
