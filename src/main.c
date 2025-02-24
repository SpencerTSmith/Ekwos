#include "core/common.h"
#include "core/linear_algebra.h"
#include "core/pool.h"
#include "core/thread_context.h"
#include "core/window.h"

#include "game/camera.h"
#include "game/entity.h"
#include "game/game.h"

#include "os/os.h"

#include "render/render_mesh.h"
#include "render/render_pipeline.h"

#include <stdio.h>

// TODO(ss); move the calculation of movment vectors out of here and into the update...
// it may be cleaner to instead just save any nessecary information into camera object and calculate
// all at once that will make it simpler to get consisten velocities even when moving diagonally
// also nice separation of concerns, this function will	JUST process input, not do any calculation
// with it
void process_input(Window *window, Camera *camera, f64 dt) {
  window_poll_events();
  f64 new_cursor_x, new_cursor_y;
  glfwGetCursorPos(window->handle, &new_cursor_x, &new_cursor_y);

  f64 x_offset = camera->sensitivity * (new_cursor_x - window->cursor_x);
  f64 y_offset = camera->sensitivity * (new_cursor_y - window->cursor_y);

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
  if (glfwGetKey(window->handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window->handle, true);

  vec3 input_direction = {0};
  // Z
  if (glfwGetKey(window->handle, GLFW_KEY_W) == GLFW_PRESS)
    input_direction = vec3_add(input_direction, camera->forward);
  if (glfwGetKey(window->handle, GLFW_KEY_S) == GLFW_PRESS)
    input_direction = vec3_sub(input_direction, camera->forward);

  // X
  if (glfwGetKey(window->handle, GLFW_KEY_D) == GLFW_PRESS)
    input_direction = vec3_add(input_direction, camera->right);
  if (glfwGetKey(window->handle, GLFW_KEY_A) == GLFW_PRESS)
    input_direction = vec3_sub(input_direction, camera->right);

  // Y
  if (glfwGetKey(window->handle, GLFW_KEY_SPACE) == GLFW_PRESS)
    input_direction = vec3_add(input_direction, camera->up);
  if (glfwGetKey(window->handle, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    input_direction = vec3_sub(input_direction, camera->up);

  input_direction = vec3_norm0(input_direction);

  vec3 camera_velocity = vec3_mul(input_direction, camera->move_speed * dt);
  camera->position = vec3_add(camera->position, camera_velocity);
}

int main(int argc, char **argv) {
  Thread_Context main_tctx;
  thread_context_init(&main_tctx);

  Game game = {0};
  game_init(&game, argc, argv);

  RND_Pipeline mesh_pipeline = rnd_pipeline_create(&game.render_context, "shaders/vert.vert.spv",
                                                   "shaders/frag.frag.spv", NULL);

  for (u32 i = 0; i < ENTITY_MAX_NUM; i++) {
    Entity *entity = NULL;
    if (i % 2 == 0) {
      entity = entity_create(&game.entity_pool, &game.render_context, &game.asset_manager,
                             ENTITY_FLAG_DEFAULT, vec3(0.f, 0.f, -2.f), vec3(0.f, 0.f, 0.f),
                             vec3(1.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f), "assets/smooth_vase.obj");
      entity->position = vec3_add(entity->position, vec3(-2.f * i, 2.f * i, -1.f * i));
      entity->scale = vec3(5.f, 5.f, 5.f);
    } else {
      entity = entity_create(&game.entity_pool, &game.render_context, &game.asset_manager,
                             ENTITY_FLAG_DEFAULT, vec3(0.f, 0.f, -2.f), vec3(0.f, 0.f, 0.f),
                             vec3(1.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f), NULL);
      entity->position = vec3_add(entity->position, vec3(2.f * i, -2.f * i, -1.f * i));
    }

    // Testing purposes
    if (i == 0 || i == 1 || i == 2 || i == 3) {
      entity_free(&game.entity_pool, entity);
    }
  }

  // Testing purposes
  entity_create(&game.entity_pool, &game.render_context, &game.asset_manager, ENTITY_FLAG_DEFAULT,
                vec3(0.f, 4.f, -2.f), vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f),
                "assets/sphere.obj");
  entity_create(&game.entity_pool, &game.render_context, &game.asset_manager, ENTITY_FLAG_DEFAULT,
                vec3(0.f, 0.f, -2.f), vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f),
                "assets/sphere.obj");

  u64 last_frame_time = get_time_ms();
  char fps_display[256];

  while (!window_should_close(&game.window)) {
    {
      u64 sleep_time = (u64)(game.target_frame_time_ms - (get_time_ms() - last_frame_time));
      if (sleep_time > 0 && sleep_time < game.target_frame_time_ms) {
        os_sleep_ms(sleep_time);
      }

      // New dt after sleeping
      game.dt = (get_time_ms() - last_frame_time) / 1000.0;

      game.fps = 1.0 / game.dt;

      // TODO(ss): Font rendering so we can just render it in game
      snprintf(fps_display, sizeof(fps_display), "%s FPS: %.2f, Frame Time: %.6fs",
               game.window.name, game.fps, game.dt);
      glfwSetWindowTitle(game.window.handle, fps_display);

      game.frame_count += 1;
      last_frame_time = get_time_ms();
    }

    process_input(&game.window, &game.camera, game.dt);

    // Update Logic
    {
      u32 entities_end = 0;
      Entity *entities = (Entity *)pool_as_array(&game.entity_pool.pool, &entities_end);
      for (u32 i = 0; i < entities_end; i++) {
        if (entities[i].flags == ENTITY_FLAG_INVALID) {
          continue;
        }

        entities[i].rotation.x += 0.10f * PI * game.dt;
        entities[i].rotation.y += 0.10f * PI * game.dt;
        entities[i].rotation.z += 0.10f * PI * game.dt;
      }
    }

    rnd_begin_frame(&game.render_context, &game.window);
    {
      f32 aspect = rnd_swap_aspect_ratio(&game.render_context);
      camera_set_perspective(&game.camera, RADIANS(90.0f), aspect, .1f, 1000.f);

      mat4 proj_view = mat4_mul(game.camera.projection, game.camera.view);
      rnd_pipeline_bind(&game.render_context, &mesh_pipeline);

      u32 entities_end = 0;
      Entity *entities = (Entity *)pool_as_array(&game.entity_pool.pool, &entities_end);
      for (u32 i = 0; i < entities_end; i++) {
        if (entities[i].flags == ENTITY_FLAG_INVALID) {
          continue;
        }
        mat4 model_transform = entity_model_transform(&entities[i]);
        mat4 clip_transform = mat4_mul(proj_view, model_transform);

        RND_Push_Constants push = {
            .clip_transform = clip_transform,
            .normal_matrix = entity_normal_matrix(&entities[i]),
        };
        rnd_push_constants(&game.render_context, &mesh_pipeline, push);

        rnd_mesh_bind(&game.render_context, entities[i].mesh_asset->data);
        rnd_mesh_draw(&game.render_context, entities[i].mesh_asset->data);
      }
    }
    rnd_end_frame(&game.render_context);
  }

  vkDeviceWaitIdle(game.render_context.logical);

  rnd_pipeline_free(&game.render_context, &mesh_pipeline);

  game_free(&game);

  thread_context_free();

  return EXT_SUCCESS;
}
