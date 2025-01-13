
#include "core/arena.h"
#include "core/common.h"
#include "core/pool.h"
#include "core/window.h"

#include "game/camera.h"
#include "game/entity.h"

#include "render/render_mesh.h"
#include "render/render_pipeline.h"
#include <stdio.h>
#include <time.h>

#define FRAME_TARGET_TIME (SECOND / FPS)
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

void process_input(Window window) {
    glfwPollEvents();

    if (glfwGetKey(window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window.handle, true);
}

typedef struct Game Game;
struct Game {
    Window window;
    RND_Context rctx;
    Camera camera;
    double fps;
    u64 frame_count;
    f64 dt;
};

int main(int argc, char **argv) {
    Game game = {0};
    Arena arena = arena_create(1024 * 100, ARENA_FLAG_DEFAULTS);

    window_create(&game.window, "Ekwos: Atavistic Chariot", WINDOW_WIDTH, WINDOW_HEIGHT);
    rnd_context_init(&arena, &game.rctx, &game.window);

    RND_Pipeline pipeline = rnd_pipeline_create(&arena, &game.rctx, "shaders/vert.vert.spv",
                                                "shaders/frag.frag.spv", NULL);
    arena_free(&arena);

    RND_Mesh mesh = {0};
    rnd_mesh_cube(&game.rctx, &mesh);

    Entity base_entity = {
        .mesh = &mesh,
        .scale = vec3(1.f, 1.f, 1.f),
        // .position.x = -.5f,
        .position.z = -2.f,
        // .rotation.z = RADIANS(45.f),
        // .rotation.x = RADIANS(30.f),
    };

#define MAX_ENTITIES 10000

    Pool entity_pool = pool_create_type(MAX_ENTITIES, Entity);
    for (i32 i = 0; i < MAX_ENTITIES; i++) {
        Entity *entity = pool_alloc(&entity_pool);
        *entity = base_entity;
    }

    clock_t last_time = clock();
    char fps_display[256];

    while (!window_should_close(&game.window)) {
        process_input(game.window);

        // Tracking fps
        clock_t current_time = clock();
        game.dt = (double)(current_time - last_time) / CLOCKS_PER_SEC;

        if (game.dt >= 1.0) {
            game.fps = game.frame_count / game.dt;

            snprintf(fps_display, sizeof(fps_display), "FPS: %.2f", game.fps);
            glfwSetWindowTitle(game.window.handle, fps_display);

            game.frame_count = 0;
            last_time = current_time;
        }

        game.frame_count += 1;

        // Updates would go here
        //
        //

        // Set camera mode
        f32 aspect = rnd_swap_aspect_ratio(&game.rctx);
        // camera_set_orthographic(&game.camera, -aspect, aspect, -1.f, 1.f, 1.f, -1.f);
        camera_set_perspective(&game.camera, RADIANS(90.0f), aspect, .1f, 10.f);
        camera_set_target(&game.camera, vec3(0.f, 1.f, 0.f), vec3(0.0f, 0.f, -2.f),
                          vec3(0.f, 1.f, 0.f));

        mat4 proj_view = mat4_mul(game.camera.projection, game.camera.view);

        rnd_begin_frame(&game.rctx, &game.window);
        rnd_pipeline_bind(&game.rctx, &pipeline);
        rnd_mesh_bind(&game.rctx, &mesh);
        Entity *entities = (Entity *)pool_as_array(&entity_pool);
        for (u32 i = 0; i < entity_pool.block_last_index; i++) {
            // entities[i].rotation.x += 0.001f * PI;
            entities[i].rotation.y += 0.001f * PI;
            // entities[i].rotation.z += 0.001f * PI;

            RND_Push_Constants push = {0};
            push.transform = mat4_mul(proj_view, entity_model_transform(&entities[i]));
            push.color = entities[i].color;
            rnd_push_constants(&game.rctx, &pipeline, push);

            rnd_mesh_draw(&game.rctx, &mesh);
        }
        rnd_end_frame(&game.rctx);
    }

    pool_free(&entity_pool);

    vkDeviceWaitIdle(game.rctx.logical);
    rnd_mesh_free(&game.rctx, &mesh);
    rnd_pipeline_free(&game.rctx, &pipeline);
    rnd_context_free(&game.rctx);
    window_free(&game.window);

    return EXT_SUCCESS;
}
