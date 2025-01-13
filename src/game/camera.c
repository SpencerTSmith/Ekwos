#include "game/camera.h"

void camera_set_orthographic(Camera *camera, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                             f32 far) {
    camera->projection = mat4_orthographic(left, right, bottom, top, near, far);
}
void camera_set_perspective(Camera *camera, f32 fov, f32 aspect_ratio, f32 z_near, f32 z_far) {
    camera->projection = mat4_perspective(fov, aspect_ratio, z_near, z_far);
}
