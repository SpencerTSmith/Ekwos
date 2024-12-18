#include "exits.h"

#include "window.h"
#include <stdio.h>

bool g_running = true;

void process_input(game::window window) {
    glfwPollEvents();

    if (glfwGetKey(window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window.handle, true);
}

int main(int argc, char **argv) {
    printf("Yo");

    game::window window = game::create_window("yo", 800, 600);

    while (g_running) {
        process_input(window);
    }

    game::free_window(&window);

    return exit_code::SUCCESS;
}
