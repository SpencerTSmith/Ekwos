#include "arena.h"
#include "args.h"
#include "common.h"
#include "window.h"

bool g_running = true;

void process_input(Window window) {
    glfwPollEvents();

    if (glfwGetKey(window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window.handle, true);
}

int main(int argc, char **argv) {
    parse_args(argc, argv);

    Arena arena = arena_create(1024 * 100);

    Window window = window_create(&arena, "yo", 800, 600);

    while (!window_should_close(window)) {
        process_input(window);
    }

    window_free(&window);

    arena_free(&arena);
    return EXT_SUCCESS;
}
