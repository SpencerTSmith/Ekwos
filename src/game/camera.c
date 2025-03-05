#include "game/camera.h"

#include "core/log.h"

const vec3 camera_up = vec3(0.f, 1.f, 0.f);

mat4 camera_get_orthographic(Camera *camera, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                             f32 far) {
  return mat4_orthographic(left, right, bottom, top, near, far);
}
mat4 camera_get_perspective(Camera *camera, f32 fov, f32 aspect_ratio, f32 z_near, f32 z_far) {
  ASSERT(z_far - z_near > 0.0f, "Camera perspective z_far must be greater than z_near");
  return mat4_perspective(fov, aspect_ratio, z_near, z_far);
}

mat4 camera_get_view(Camera *camera) {
  // And all negatives because z grows negative into the world
  vec3 forward = {
      .x = -cosf(RADIANS(camera->yaw)) * cosf(RADIANS(camera->pitch)),
      .y = -sinf(RADIANS(camera->pitch)),
      .z = -sinf(RADIANS(camera->yaw)) * cosf(RADIANS(camera->pitch)),
  };
  forward = vec3_norm0(forward);

  mat4 view = mat4_look_direction(camera->position, forward, camera_up);
  return view;
}

void camera_get_directions(Camera *camera, vec3 *out_foreward, vec3 *out_up, vec3 *out_right) {
  vec3 forward = {
      .x = -cosf(RADIANS(camera->yaw)) * cosf(RADIANS(camera->pitch)),
      .y = -sinf(RADIANS(camera->pitch)),
      .z = -sinf(RADIANS(camera->yaw)) * cosf(RADIANS(camera->pitch)),
  };
  forward = vec3_norm0(forward);

  *out_foreward = forward;
  *out_up = camera_up;
  *out_right = vec3_cross(forward, camera_up);
}

// void camera_set_direction(Camera *camera, vec3 position, vec3 direction, vec3 up) {
//   ASSERT(vec3_len(direction) != 0.0f, "Camera direction is zero vector!");
//   ASSERT(vec3_len(up) != 0.0f, "Camera direction is zero vector!");
//
//   camera->view = mat4_look_direction(position, direction, up);
//   camera->forward = direction;
//   camera->position = position;
//   camera->up = up;
//   camera->right = vec3_cross(camera->forward, camera->up);
// }
//
// void camera_set_target(Camera *camera, vec3 position, vec3 target, vec3 up) {
//   ASSERT(vec3_len(vec3_sub(position, target)) != 0.0f, "Camera direction is zero vector!");
//   ASSERT(vec3_len(up) != 0.0f, "Camera direction is zero vector!");
//
//   camera->view = mat4_look_at(position, target, up);
//   camera->forward = vec3_sub(position, target);
//   camera->position = position;
//   camera->up = up;
//   camera->right = vec3_cross(camera->forward, camera->up);
// }
