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

    Game game = {0};
    Arena arena = arena_create(1024 * 100);
    window_create(&game.window, "Ekwos: Atavistic Chariot", 800, 600);
    render_context_init(&arena, &game.rctx, game.window.handle);

    Render_Pipeline pipeline = render_pipeline_create(&arena, &game.rctx, "shaders/vert.vert.spv",
                                                      "shaders/frag.frag.spv", NULL);
    arena_free(&arena);

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
            for (u32 i = 0; i < 4; i++) {
                Push_Constants push = {0};
                push.offset = vec2(0.0f, -0.4f + i * 0.25f);
                push.color = vec3(0.0f - i * .2f, 0.8f - i * .2f, 0.2f + 0.2f * i);
                render_push_constants(&game.rctx, &pipeline, push);

                render_mesh_draw(&game.rctx, &mesh);
            }
            render_end_frame(&game.rctx);
        }
    }

    vkDeviceWaitIdle(game.rctx.logical);
    render_mesh_free(&game.rctx, &mesh);
    render_pipeline_free(&game.rctx, &pipeline);
    render_context_free(&game.rctx);
    window_free(&game.window);

    return EXT_SUCCESS;
}
