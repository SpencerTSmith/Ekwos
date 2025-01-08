#ifndef WINDOW_H
#define WINDOW_H

#include "core/common.h"

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
};

void window_create(Window *window, const char *name, int width, int height);
void window_free(Window *window);
bool window_should_close(Window window);

#endif // WINDOW_H
