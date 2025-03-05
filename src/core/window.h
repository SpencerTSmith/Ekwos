#ifndef WINDOW_H
#define WINDOW_H

#include "core/common.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <stdbool.h>

enum Window_Constants {
  WINDOW_DEFAULT_WIDTH = 1200,
  WINDOW_DEFAULT_HEIGHT = 900,
};

typedef struct Window Window;
struct Window {
  union {
    u32 width, w;
  };
  union {
    u32 height, h;
  };

  GLFWwindow *handle;
  char title[256];
  bool resized;

  f64 cursor_x, cursor_y;
};

void window_init(Window *window, char *name, u32 width, u32 height);
void window_free(Window *window);

void window_set_title(Window *window, char *title);
void window_update_title_fps_dt(Window *window, f64 fps, f64 dt);

void window_poll_events(void);

bool window_should_close(Window *window);
void window_set_to_close(Window *window);

#endif // WINDOW_H
