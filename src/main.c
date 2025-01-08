#include "core/arena.h"
#include "core/common.h"
#include "render/render_mesh.h"
#include "render/render_pipeline.h"
#include "window.h"

bool g_running = true;

void process_input(Window window) {
    glfwPollEvents();

    if (glfwGetKey(window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window.handle, true);
}

typedef struct Game Game;
struct Game {
    Window window;
    Render_Context rctx;
};

int main(int argc, char **argv) {
    Arena arena = arena_create(1024 * 100);

    Game game = {0};
    window_create(&game.window, "Ekwos: Atavistic Chariot", 800, 600);
    render_context_init(&arena, &game.rctx, game.window.handle);

    Pipeline_Config config =
        default_pipeline_config(render_swap_width(&game.rctx), render_swap_height(&game.rctx));
    Render_Pipeline pipeline = render_pipeline_create(&arena, &game.rctx, "shaders/vert.vert.spv",
                                                      "shaders/frag.frag.spv", &config);
    Vertex verts[] = {
        {.position = {.x = 0.0f, .y = -0.5f}, .color = {.r = 1.0f, .g = 0.0f, .b = 0.0f}},
        {.position = {.x = 0.5f, .y = 0.5f}, .color = {.r = 0.0f, .g = 1.0f, .b = 0.0f}},
        {.position = {.x = -0.5f, .y = 0.5f}, .color = {.r = 0.0f, .g = 0.0f, .b = 1.0f}},
    };
    u32 verts_count = 3;
    Render_Mesh mesh = {0};
    render_mesh_init(&game.rctx, &mesh, verts, verts_count);

    while (!window_should_close(game.window)) {
        process_input(game.window);
        if (render_begin_frame(&game.rctx, &game.window)) {
            render_pipeline_bind(&game.rctx, &pipeline);
            render_mesh_bind(&game.rctx, &mesh);
            render_mesh_draw(&game.rctx, &mesh);
            render_end_frame(&game.rctx);
        }
    }

    vkDeviceWaitIdle(game.rctx.logical);
    render_mesh_free(&game.rctx, &mesh);
    render_pipeline_free(&game.rctx, &pipeline);
    render_context_free(&game.rctx);
    window_free(&game.window);

    arena_free(&arena);
    return EXT_SUCCESS;
}
