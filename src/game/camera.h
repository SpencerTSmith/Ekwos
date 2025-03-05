#ifndef CAMERA_H
#define CAMERA_H

#include "core/linear_algebra.h"

// No roll for the forseeable future so this is (0.f, 1.f, 0.f)
extern const vec3 CAMERA_UP;

typedef struct Camera Camera;
struct Camera {
  vec3 position;
  f32 yaw, pitch;
  f32 move_speed, sensitivity;
};

// Not nessecary to pass in Camera now, but we may want to store more info in the camera struct in
// future
mat4 camera_get_orthographic(Camera *camera, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                             f32 far);
mat4 camera_get_perspective(Camera *camera, f32 fov, f32 aspect_ratio, f32 z_near, f32 z_far);

mat4 camera_get_view(Camera *camera);
void camera_get_directions(Camera *camera, vec3 *out_forward, vec3 *out_up, vec3 *out_right);

// TODO(ss): rewrite this using new way
// void camera_set_direction(Camera *camera, vec3 position, vec3 direction, vec3 up);
// void camera_set_target(Camera *camera, vec3 position, vec3 target, vec3 up);
#endif // CAMERA_H
