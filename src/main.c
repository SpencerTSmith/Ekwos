#include "exit.h"

#include "window.h"
#include <stdio.h>

bool g_running = true;

void process_input(window window) {
    glfwPollEvents();

    if (glfwGetKey(window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window.handle, true);
}

int main(int argc, char **argv) {
    printf("Yo");

    window window = create_window("yo", 800, 600);

    while (g_running) {
        process_input(window);
    }

    free_window(&window);

    return EXT_SUCCESS;
}
