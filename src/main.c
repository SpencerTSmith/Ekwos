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

// TODO(ss); move the calculation of movment vectors out of here and into the update...
// it may be cleaner to instead just save any nessecary information into camera object and calculate
// all at once that will make it simpler to get consisten velocities even when moving diagonally
// also nice separation of concerns, this function will	JUST process input, not do any calculation
// with it
void process_input(Window *window, Camera *camera, f64 dt) {
  window_poll_events();

  // Hold ctrl to
  f64 new_cursor_x, new_cursor_y;
  glfwGetCursorPos(window->handle, &new_cursor_x, &new_cursor_y);

  f64 x_offset = new_cursor_x - window->cursor_x;
  f64 y_offset = new_cursor_y - window->cursor_y;

  // No dt, remember this is not a rate but a direct distance
  camera->yaw += camera->sensitivity * x_offset;
  camera->pitch += camera->sensitivity * y_offset;
  camera->pitch = CLAMP(camera->pitch, -89.f, 89.f);

  window->cursor_x = new_cursor_x;
  window->cursor_y = new_cursor_y;

  if (glfwGetKey(window->handle, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window->handle, true);

  vec3 camera_forward;
  vec3 camera_up;
  vec3 camera_right;
  camera_get_directions(camera, &camera_forward, &camera_up, &camera_right);

  vec3 input_direction = {0};
  // Z, forward
  if (glfwGetKey(window->handle, GLFW_KEY_W) == GLFW_PRESS)
    input_direction = vec3_add(input_direction, camera_forward);
  if (glfwGetKey(window->handle, GLFW_KEY_S) == GLFW_PRESS)
    input_direction = vec3_sub(input_direction, camera_forward);

  // X, strafe
  if (glfwGetKey(window->handle, GLFW_KEY_D) == GLFW_PRESS)
    input_direction = vec3_add(input_direction, camera_right);
  if (glfwGetKey(window->handle, GLFW_KEY_A) == GLFW_PRESS)
    input_direction = vec3_sub(input_direction, camera_right);

  // Y, vertical
  if (glfwGetKey(window->handle, GLFW_KEY_SPACE) == GLFW_PRESS)
    input_direction = vec3_add(input_direction, camera_up);
  if (glfwGetKey(window->handle, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    input_direction = vec3_sub(input_direction, camera_up);

  input_direction = vec3_norm0(input_direction);

  vec3 camera_velocity = vec3_mul(input_direction, camera->move_speed * dt);
  camera->position = vec3_add(camera->position, camera_velocity);
}

// MAIN!!!
int main(int argc, char **argv) {
  Thread_Context main_tctx;
  thread_context_init(&main_tctx);

  Game game = {0};
  game_init(&game, argc, argv);

  RND_Buffer global_uniform_buffer = rnd_buffer_make_GUBO(
      &game.render_context, sizeof(RND_Global_UBO), game.render_context.swap.frames_in_flight);

  {
    for (u32 i = 0; i < ENTITY_MAX_NUM / 10; i++) {
      Entity *entity = NULL;
      if (i % 3 == 0) {
        entity = entity_make(&game.entity_pool, &game.render_context, &game.asset_manager,
                             ENTITY_FLAG_DEFAULT, vec3(0.f, 0.f, -2.f), vec3(0.f, 0.f, 0.f),
                             vec3(1.f, 1.f, 1.f), NULL);
        entity->position = vec3_add(entity->position, vec3(2.f * i, -2.f * i, -1.f * i));
      } else if (i % 3 == 1) {
        entity = entity_make(&game.entity_pool, &game.render_context, &game.asset_manager,
                             ENTITY_FLAG_DEFAULT, vec3(0.f, 0.f, -2.f), vec3(0.f, 0.f, 0.f),
                             vec3(1.f, 1.f, 1.f), "assets/smooth_vase.obj");
        entity->position = vec3_add(entity->position, vec3(-2.f * i, 2.f * i, -1.f * i));
        entity->scale = vec3(5.f, 5.f, 5.f);
      } else if (i % 3 == 2) {
        entity = entity_make(&game.entity_pool, &game.render_context, &game.asset_manager,
                             ENTITY_FLAG_DEFAULT, vec3(0.f, 0.f, -2.f), vec3(0.f, 0.f, 0.f),
                             vec3(1.f, 1.f, 1.f), "assets/flat_vase.obj");
        entity->position = vec3_add(entity->position, vec3(0.f, 2.f * i, -1.f * i));
        entity->scale = vec3(5.f, 5.f, 5.f);
      }

      // Testing purposes
      if ((i <= 5) || (i >= 10 && i <= 15)) {
        entity_free(&game.entity_pool, &game.render_context, &game.asset_manager, entity);
      }
    }

    // Testing purposes
    entity_make(&game.entity_pool, &game.render_context, &game.asset_manager, ENTITY_FLAG_DEFAULT,
                vec3(0.f, 4.f, -2.f), vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f), "assets/f22.obj");
    entity_make(&game.entity_pool, &game.render_context, &game.asset_manager, ENTITY_FLAG_DEFAULT,
                vec3(0.f, 0.f, -2.f), vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f),
                "assets/sphere.obj");
    entity_make(&game.entity_pool, &game.render_context, &game.asset_manager, ENTITY_FLAG_DEFAULT,
                vec3(0.f, -4.f, -2.f), vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f), "assets/crab.obj");
    entity_make(&game.entity_pool, &game.render_context, &game.asset_manager, ENTITY_FLAG_DEFAULT,
                vec3(4.f, -4.f, -5.f), vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f),
                "assets/colored_cube.obj");
    entity_make(&game.entity_pool, &game.render_context, &game.asset_manager, ENTITY_FLAG_DEFAULT,
                vec3(4.f, -4.f, -5.f), vec3(0.f, 0.f, 0.f), vec3(1.f, 1.f, 1.f),
                "assets/colored_cube.obj");

    // TODO(ss): make sure we will reuse the asset spot freed
    Entity *to_free = entity_make(&game.entity_pool, &game.render_context, &game.asset_manager,
                                  ENTITY_FLAG_DEFAULT, vec3(4.f, -4.f, -5.f), vec3(0.f, 0.f, 0.f),
                                  vec3(1.f, 1.f, 1.f), "assets/f117.obj");
    entity_free(&game.entity_pool, &game.render_context, &game.asset_manager, to_free);
  }

  // First frame time
  u64 last_frame_time = get_time_ns();
  while (!window_should_close(&game.window)) {
    // Tick rate stuff
    {
      u64 sleep_time = game.target_frame_time_ns - (get_time_ns() - last_frame_time);
      if (sleep_time < game.target_frame_time_ns) {
        os_sleep_ns(sleep_time);
      }

      // New dt after sleeping
      game.dt_s = (get_time_ns() - last_frame_time) / (double)NSEC_PER_SEC;

      game.fps = 1.0 / game.dt_s;

      // TODO(ss): Font rendering so we can just render it in game
      if (game.frame_count % (u64)game.fps == 0)
        window_update_title_fps_dt(&game.window, game.fps, game.dt_s);

      game.frame_count++;
      last_frame_time = get_time_ns();
    }

    process_input(&game.window, &game.camera, game.dt_s);

    // Update Logic
    {
      u32 entities_end = 0;
      Entity *entities = (Entity *)pool_as_array(&game.entity_pool.pool, &entities_end);
      for (u32 i = 0; i < entities_end; i++) {
        if (entities[i].flags == ENTITY_FLAG_INVALID) {
          continue;
        }

        // entities[i].rotation.x += 0.10f * PI * game.dt;
        entities[i].rotation.y += 0.10f * PI * game.dt_s;
        entities[i].rotation.z += 0.10f * PI * game.dt_s;
      }
    }

    rnd_begin_frame(&game.render_context, &game.window);
    {
      f32 aspect = rnd_swap_aspect_ratio(&game.render_context);

      RND_Global_UBO ubo = {
          .projection = camera_get_perspective(&game.camera, RADIANS(90.0f), aspect, .1f, 1000.f),
          .view = camera_get_view(&game.camera),
          .global_light_direction = vec3(1.f, 1.f, 1.f),
      };
      rnd_buffer_write_item_index(&global_uniform_buffer, &ubo,
                                  game.render_context.swap.current_frame_idx);

      mat4 proj_view = mat4_mul(ubo.projection, ubo.view);

      rnd_pipeline_bind(&game.render_context, &game.render_context.pipelines[RND_PIPELINE_MESH]);

      u32 entities_end = 0;
      Entity *entities = (Entity *)pool_as_array(&game.entity_pool.pool, &entities_end);
      for (u32 i = 0; i < entities_end; i++) {
        if (entities[i].flags == ENTITY_FLAG_INVALID) {
          continue;
        }
        mat4 model_transform = entity_model_mat4(&entities[i]);
        mat4 clip_transform = mat4_mul(proj_view, model_transform);

        RND_Push_Constants push = {
            .clip_transform = clip_transform,
            .normal_matrix = entity_normal_mat4(&entities[i]),
        };
        rnd_pipeline_push_constants(&game.render_context,
                                    &game.render_context.pipelines[RND_PIPELINE_MESH], push);

        rnd_mesh_bind(&game.render_context, entities[i].mesh_asset->data);
        rnd_mesh_draw(&game.render_context, entities[i].mesh_asset->data);
      }
    }
    rnd_end_frame(&game.render_context);

    arena_clear(&game.frame_arena);
  }

  vkDeviceWaitIdle(game.render_context.logical);

  rnd_buffer_free(&game.render_context, &global_uniform_buffer);

  game_free(&game);

  thread_context_free();

  return EXT_SUCCESS;
}
