#include "window.h"

namespace game {

window create_window(const char *name, int width, int height) {
    window window = {};

    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window.w = width;
    window.h = height;

    window.handle = glfwCreateWindow(width, height, name, nullptr, nullptr);
    return window;
}

void free_window(window *window) {
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}

bool window_should_close(window window) { return glfwWindowShouldClose(window.handle); }

} // namespace game
