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
#include <unistd.h>

#define TARGET_FPS 60.0
#define TARGET_FRAME_TIME_MS (1000.0 / TARGET_FPS)

// TODO(ss); move the calculation of movment vectors out of here and into the update...
// it may be cleaner to instead just save any nessecary information into camera object and calculate
// all at once that will make it simpler to get consisten velocities even when moving diagonally
// also nice separation of concerns, this function will	JUST process input, not do any calculation
// with it
void process_input(Window *window, Camera *camera, f64 dt) {
  f64 new_cursor_x, new_cursor_y;
  glfwGetCursorPos(window->handle, &new_cursor_x, &new_cursor_y);

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
  if (glfwGetKey(window->handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window->handle, true);

  vec3 input_direction = {0};
  if (glfwGetKey(window->handle, GLFW_KEY_W) == GLFW_PRESS)
    input_direction = vec3_add(input_direction, camera->forward);
  if (glfwGetKey(window->handle, GLFW_KEY_S) == GLFW_PRESS)
    input_direction = vec3_sub(input_direction, camera->forward);

  if (glfwGetKey(window->handle, GLFW_KEY_D) == GLFW_PRESS)
    input_direction = vec3_add(input_direction, camera->right);
  if (glfwGetKey(window->handle, GLFW_KEY_A) == GLFW_PRESS)
    input_direction = vec3_sub(input_direction, camera->right);

  // HACK(ss): The only way to make sure no div by 0?
  if (vec3_len(input_direction) > 0.0f)
    input_direction = vec3_norm(input_direction);

  vec3 camera_velocity = vec3_mul(input_direction, 2.f * dt);
  camera->position = vec3_add(camera->position, camera_velocity);
  vec3_print(camera->position);

  if (glfwGetKey(window->handle, GLFW_KEY_SPACE) == GLFW_PRESS) {
    camera->position.y += 1.f * dt;
  }
  if (glfwGetKey(window->handle, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    camera->position.y -= 1.f * dt;
  }
}

int main(int argc, char **argv) {
  Thread_Context main_tctx;
  thread_context_init(&main_tctx);

  Game game = {0};
  game_init(&game, argc, argv);

  RND_Pipeline mesh_pipeline = rnd_pipeline_create(&game.render_context, "shaders/vert.vert.spv",
                                                   "shaders/frag.frag.spv", NULL);

  for (u32 i = 0; i < game.entity_pool.pool.block_capacity; i++) {
    Entity *entity =
        entity_create(&game.entity_pool, &game.render_context, &game.asset_manager,
                      EK_ENTITY_FLAG_DEFAULTS, vec3(0.f, 0.f, -4.f), vec3(0.f, 0.f, 0.f),
                      vec3(1.f, 1.f, 1.f), vec3(0.f, 0.f, 0.f), "assets/cube.obj");

    entity->scale = vec3(1.f, 1.f, 1.f);
  }

  u64 last_time = get_time_ms();
  char fps_display[256];

  while (!window_should_close(&game.window)) {
    {
      u64 sleep_time = TARGET_FRAME_TIME_MS - (get_time_ms() - last_time);
      if (sleep_time > 0 && sleep_time < TARGET_FRAME_TIME_MS) {
        usleep(sleep_time * 1000);
      }

      u64 current_time = get_time_ms();

      // New dt after sleeping
      game.dt = (current_time - last_time) / 1000.0;

      game.fps = 1 / game.dt;

      // TODO(ss): Font rendering so we can just render it in game
      snprintf(fps_display, sizeof(fps_display), "%s FPS: %.2f", game.window.name, game.fps);
      glfwSetWindowTitle(game.window.handle, fps_display);

      game.frame_count += 1;
      last_time = current_time;
    }

    process_input(&game.window, &game.camera, game.dt);

    // Updates would go here
    //
    // // // //

    // Render
    {
      f32 aspect = rnd_swap_aspect_ratio(&game.render_context);
      // camera_set_orthographic(&game.camera, -aspect, aspect, -1.f, 1.f, 1.f, -1.f);
      camera_set_perspective(&game.camera, RADIANS(90.0f), aspect, .1f, 10.f);

      mat4 proj_view = mat4_mul(game.camera.projection, game.camera.view);

      rnd_begin_frame(&game.render_context, &game.window);

      rnd_pipeline_bind(&game.render_context, &mesh_pipeline);

      u32 entities_count = 0;
      Entity *entities = (Entity *)pool_as_array(&game.entity_pool.pool, &entities_count);
      for (u32 i = 0; i < game.entity_pool.pool.block_last_index; i++) {
        if (entities[i].id == ENTITY_INVALID_ID) {
          continue;
        }

        entities[i].rotation.x += 0.001f * PI;
        entities[i].rotation.y += 0.001f * PI;
        entities[i].rotation.z += 0.001f * PI;
        mat4 e_transform = mat4_mul(proj_view, entity_model_transform(&entities[i]));

        RND_Push_Constants push = {0};
        push.transform = e_transform;
        push.color = entities[i].color;
        rnd_push_constants(&game.render_context, &mesh_pipeline, push);

        rnd_mesh_bind(&game.render_context, entities->mesh_asset.data);
        rnd_mesh_draw(&game.render_context, entities->mesh_asset.data);
      }

      rnd_end_frame(&game.render_context);
    }

    poll_events();
  }

  vkDeviceWaitIdle(game.render_context.logical);

  rnd_pipeline_free(&game.render_context, &mesh_pipeline);

  game_free(&game);

  thread_context_free();

  return EXT_SUCCESS;
}
