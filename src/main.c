
#include "core/arena.h"
#include "core/common.h"
#include "core/pool.h"
#include "core/window.h"

#include "game/entity.h"

#include "render/render_mesh.h"
#include "render/render_pipeline.h"
#include <stdio.h>
#include <time.h>

#define FRAME_TARGET_TIME (SECOND / FPS)

void process_input(Window window) {
    glfwPollEvents();

    if (glfwGetKey(window.handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window.handle, true);
}

typedef struct Game Game;
struct Game {
    Window window;
    RND_Context rctx;
};

int main(int argc, char **argv) {
    Game game = {0};
    Arena arena = arena_create(1024 * 100, ARENA_FLAG_DEFAULTS);
    window_create(&game.window, "Ekwos: Atavistic Chariot", 800, 600);
    rnd_context_init(&arena, &game.rctx, &game.window);

    RND_Pipeline pipeline = rnd_pipeline_create(&arena, &game.rctx, "shaders/vert.vert.spv",
                                                "shaders/frag.frag.spv", NULL);
    arena_free(&arena);

    RND_Mesh mesh = {0};
    rnd_mesh_cube(&game.rctx, &mesh);

    Entity base_entity = {
        .mesh = &mesh,
        .position.z = 0.5f,
        .scale = vec3(0.5f, 0.5f, 0.5f),
    };

#define MAX_ENTITIES 1

    Pool entity_pool = pool_create_type(MAX_ENTITIES, Entity);
    for (u32 i = 0; i < MAX_ENTITIES; i++) {
        Entity *entity = pool_alloc(&entity_pool);
        *entity = base_entity;
    }

    clock_t last_time = clock();
    double fps = 0.0;
    char fps_display[256];
    u64 frame_count = 0;

    while (!window_should_close(&game.window)) {
        process_input(game.window);

        clock_t current_time = clock();
        double dt = (double)(current_time - last_time) / CLOCKS_PER_SEC;

        if (dt >= 1.0) {
            fps = frame_count / dt;

            snprintf(fps_display, sizeof(fps_display), "FPS: %.2f", fps);
            glfwSetWindowTitle(game.window.handle, fps_display);

            frame_count = 0;
            last_time = current_time;
        }

        frame_count += 1;

        rnd_begin_frame(&game.rctx, &game.window);
        rnd_pipeline_bind(&game.rctx, &pipeline);
        rnd_mesh_bind(&game.rctx, &mesh);
        Entity *entities = (Entity *)pool_as_array(&entity_pool);
        for (u32 i = 0; i < entity_pool.block_last_index; i++) {
            entities[i].rotation.x += 0.00001f * 2 * PI;
            entities[i].rotation.y += 0.00001f * 2 * PI;
            entities[i].rotation.z += 0.00001f * 2 * PI;
            // float rand_x = 2.0f * ((float)rand() / (float)RAND_MAX - 0.5f);
            // float rand_y = 2.0f * ((float)rand() / (float)RAND_MAX - 0.5f);

            RND_Push_Constants push = {0};
            push.transform = entity_transform(&entities[i]);
            // push.transform = mat4_identity();
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
