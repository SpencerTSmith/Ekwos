#ifndef CAMERA_H
#define CAMERA_H

#include "core/linear_algebra.h"
typedef struct Camera Camera;
struct Camera {
    mat4 projection;
};

void camera_set_orthographic(Camera *camera, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                             f32 far);
void camera_set_perspective(Camera *camera, f32 fov, f32 aspect_ratio, f32 z_near, f32 z_far);

#endif // CAMERA_H
