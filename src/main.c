#include "core/common.h"
#include "core/linear_algebra.h"
#include "core/pool.h"
#include "core/thread_context.h"
#include "core/window.h"

#include "game/camera.h"
#include "game/entity.h"
#include "game/game.h"

#include "render/render_mesh.h"
#include "render/render_pipeline.h"
#include <stdio.h>
#include <time.h>

#define FRAME_TARGET_TIME (SECOND / FPS)

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 900

#define MAX_ENTITIES 1000

static bool first_mouse = true;

// TODO(ss); move the calculation of movment vectors out of here and into the update...
// it may be cleaner to instead just save any nessecary information into camera object and calculate
// all at once that will make it simpler to get consisten velocities even when moving diagonally
// also nice separation of concerns, this function will	JUST process input, not do any calculation
// with it
void process_input(Window *window, Camera *camera, f64 dt) {
    f64 new_cursor_x, new_cursor_y;
    glfwGetCursorPos(window->handle, &new_cursor_x, &new_cursor_y);

    if (first_mouse) {
        window->cursor_x = new_cursor_x;
        window->cursor_x = new_cursor_x;
        first_mouse = false;
        return;
    }

    f32 x_offset = .1f * (new_cursor_x - window->cursor_x);
    f32 y_offset = .1f * (new_cursor_y - window->cursor_y);

    window->cursor_x = new_cursor_x;
    window->cursor_y = new_cursor_y;

    camera->yaw += x_offset;
    camera->pitch += y_offset;
    camera->pitch = CLAMP(camera->pitch, -89.f, 89.f);

    vec3 forward;
    forward.x = -cosf(RADIANS(camera->yaw)) * cosf(RADIANS(camera->pitch));
    forward.y = -sinf(RADIANS(camera->pitch));
    forward.z = -sinf(RADIANS(camera->yaw)) * cosf(RADIANS(camera->pitch));
    forward = vec3_norm(forward);

    camera_set_direction(camera, camera->position, forward, vec3(0.0f, 1.0f, 0.0f));

    vec3 velocity = {0};
    if (glfwGetKey(window->handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window->handle, true);

    if (glfwGetKey(window->handle, GLFW_KEY_W) == GLFW_PRESS) {
        velocity = vec3_mul(camera->forward, .10f * dt);
        camera->position = vec3_add(camera->position, velocity);
    }
    if (glfwGetKey(window->handle, GLFW_KEY_S) == GLFW_PRESS) {
        velocity = vec3_mul(camera->forward, .10f * dt);
        camera->position = vec3_sub(camera->position, velocity);
    }

    if (glfwGetKey(window->handle, GLFW_KEY_D) == GLFW_PRESS) {
        velocity = vec3_mul(camera->right, .10f * dt);
        camera->position = vec3_add(camera->position, velocity);
    }
    if (glfwGetKey(window->handle, GLFW_KEY_A) == GLFW_PRESS) {
        velocity = vec3_mul(camera->right, .10f * dt);
        camera->position = vec3_sub(camera->position, velocity);
    }

    if (glfwGetKey(window->handle, GLFW_KEY_SPACE) == GLFW_PRESS) {
        camera->position.y += .10f * dt;
    }
    if (glfwGetKey(window->handle, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        camera->position.y -= .10f * dt;
    }
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    Thread_Context main_tctx;
    thread_context_init(&main_tctx);
    Game game = {0};

    window_init(&game.window, "Ekwos: Atavistic Chariot", WINDOW_WIDTH, WINDOW_HEIGHT);
    rnd_context_init(&game.rctx, &game.window);

    RND_Pipeline mesh_pipeline =
        rnd_pipeline_create(&game.rctx, "shaders/vert.vert.spv", "shaders/frag.frag.spv", NULL);

    // Initial camera options
    camera_set_perspective(&game.camera, RADIANS(90.f), (f32)WINDOW_WIDTH / WINDOW_HEIGHT, .1f,
                           10.f);
    camera_set_direction(&game.camera, vec3(0.f, 0.f, 0.f), vec3(0.0f, 0.f, -1.f),
                         vec3(0.f, 1.f, 0.f));

    RND_Mesh mesh = {0};
    rnd_mesh_cube(&game.rctx, &mesh);

    Entity_Pool entity_pool = entity_pool_create(MAX_ENTITIES);
    for (u32 i = 0; i < entity_pool.pool.block_capacity; i++) {
        Entity *entity =
            entity_create(&entity_pool, EK_ENTITY_FLAG_DEFAULTS, vec3(0.f, 0.f, -2.f),
                          vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f), &mesh);
        entity->rotation.z += 0.001f * i * PI;
        entity->rotation.x -= 0.001f * i * PI;
    }

    clock_t last_time = clock();
    char fps_display[256];

    while (!window_should_close(&game.window)) {
        {
            clock_t current_time = clock();
            game.dt = (double)(current_time - last_time) / CLOCKS_PER_SEC;

            if (game.dt >= 0.2) {
                game.fps = game.frame_count / game.dt;

                snprintf(fps_display, sizeof(fps_display), "%s FPS: %.2f", game.window.name,
                         game.fps);
                glfwSetWindowTitle(game.window.handle, fps_display);

                game.frame_count = 0;
                last_time = current_time;
            }

            game.frame_count += 1;
        }

        process_input(&game.window, &game.camera, game.dt);

        // Updates would go here
        //
        // // // //

        // Render
        {
            f32 aspect = rnd_swap_aspect_ratio(&game.rctx);
            // camera_set_orthographic(&game.camera, -aspect, aspect, -1.f, 1.f, 1.f, -1.f);
            camera_set_perspective(&game.camera, RADIANS(90.0f), aspect, .1f, 10.f);

            mat4 proj_view = mat4_mul(game.camera.projection, game.camera.view);

            rnd_begin_frame(&game.rctx, &game.window);
            rnd_pipeline_bind(&game.rctx, &mesh_pipeline);
            rnd_mesh_bind(&game.rctx, &mesh);
            Entity *entities = (Entity *)pool_as_array(&entity_pool.pool);
            for (u32 i = 0; i < entity_pool.pool.block_last_index; i++) {
                // entities[i].rotation.x += 0.001f * PI;
                entities[i].rotation.y += 0.001f * PI;
                // entities[i].rotation.z += 0.001f * PI;

                RND_Push_Constants push = {0};
                push.transform = mat4_mul(proj_view, entity_model_transform(&entities[i]));
                push.color = entities[i].color;
                rnd_push_constants(&game.rctx, &mesh_pipeline, push);

                rnd_mesh_draw(&game.rctx, &mesh);
            }
            rnd_end_frame(&game.rctx);
        }

        poll_events();
    }

    entity_pool_free(&entity_pool);

    vkDeviceWaitIdle(game.rctx.logical);
    rnd_mesh_free(&game.rctx, &mesh);
    rnd_pipeline_free(&game.rctx, &mesh_pipeline);
    rnd_context_free(&game.rctx);
    window_free(&game.window);

    thread_context_free();

    return EXT_SUCCESS;
}
