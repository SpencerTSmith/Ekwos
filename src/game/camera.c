#include "game/camera.h"
#include <assert.h>

void camera_set_orthographic(Camera *camera, f32 left, f32 right, f32 bottom, f32 top, f32 near,
                             f32 far) {
    camera->projection = mat4_orthographic(left, right, bottom, top, near, far);
}
void camera_set_perspective(Camera *camera, f32 fov, f32 aspect_ratio, f32 z_near, f32 z_far) {
    camera->projection = mat4_perspective(fov, aspect_ratio, z_near, z_far);
}

void camera_set_direction(Camera *camera, vec3 position, vec3 direction, vec3 up) {
    assert(vec3_len(direction) != 0.0f && "Camera direction is zero vector!");
    camera->view = mat4_look_direction(position, direction, up);
    camera->forward = direction;
    camera->position = position;
    camera->up = up;
    camera->right = vec3_cross(camera->forward, camera->up);
}

void camera_set_target(Camera *camera, vec3 position, vec3 target, vec3 up) {
    assert(vec3_len(vec3_sub(position, target)) != 0.0f && "Camera direction is zero vector!");
    camera->view = mat4_look_at(position, target, up);
    camera->forward = vec3_sub(position, target);
    camera->position = position;
    camera->up = up;
    camera->right = vec3_cross(camera->forward, camera->up);
}

// void camera_set_xyz(Camera *camera, vec3 position, vec3 rotation) {}
