#ifndef CAMERA_H
#define CAMERA_H

#include "core/linear_algebra.h"
typedef struct Camera Camera;
struct Camera {
    mat4 projection, view;
    f32 sensitivity;
    f32 yaw, pitch;
    vec3 position, direction, up;
};

void camera_init(Camera *camera, vec3 position, vec3 direction, vec3 up);

void camera_set_orthographic(Camera *camera, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                             f32 far);
void camera_set_perspective(Camera *camera, f32 fov, f32 aspect_ratio, f32 z_near, f32 z_far);

void camera_set_direction(Camera *camera, vec3 position, vec3 direction, vec3 up);
void camera_set_target(Camera *camera, vec3 position, vec3 target, vec3 up);
// void camera_set_xyz(Camera *camera, vec3 position, vec3 rotation);
#endif // CAMERA_H
