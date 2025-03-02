#ifndef CAMERA_H
#define CAMERA_H

#include "core/linear_algebra.h"

// TODO(ss): probably just store only position, yaw, pitch... calculate everything else on demand
// always in sync that way
typedef struct Camera Camera;
struct Camera {
  mat4 view;
  vec3 forward, right, up;
  vec3 position;
  f32 yaw, pitch;
  f32 move_speed, sensitivity;
};

mat4 camera_get_orthographic(Camera *camera, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                             f32 far);
mat4 camera_get_perspective(Camera *camera, f32 fov, f32 aspect_ratio, f32 z_near, f32 z_far);

void camera_set_direction(Camera *camera, vec3 position, vec3 direction, vec3 up);
void camera_set_target(Camera *camera, vec3 position, vec3 target, vec3 up);
#endif // CAMERA_H
