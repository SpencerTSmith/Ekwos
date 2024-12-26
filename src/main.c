#include "arena.h"
#include "common.h"
#include "shader.h"
#include "window.h"

#include <stdio.h>

bool g_running = true;

void process_input(Window window) {
    glfwPollEvents();

    if (glfwGetKey(window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window.handle, true);
}

int main(int argc, char **argv) {
    Arena arena = {0};
    arena_create(&arena, 1024 * 10);

    Window window = window_create(&arena, "yo", 800, 600);

    ShaderData frag_shader = read_shader_file(&arena, "./src/shaders/first_frag.frag.spv");
    ShaderData vert_shader = read_shader_file(&arena, "./src/shaders/first_vert.vert.spv");

    while (g_running) {
        process_input(window);
    }

    window_free(&window);

    arena_free(&arena);
    return EXT_SUCCESS;
}
