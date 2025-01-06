#include "matrix.h"

#include <math.h>

mat4_t mat4_identity(void) {
    mat4_t i = {.m = {
                    {1, 0, 0, 0},
                    {0, 1, 0, 0},
                    {0, 0, 1, 0},
                    {0, 0, 0, 1},
                }};
    return i;
}

mat4_t mat4_make_scale(float sx, float sy, float sz) {
    mat4_t s = mat4_identity();
    s.m[0][0] = sx;
    s.m[1][1] = sy;
    s.m[2][2] = sz;

    return s;
}

mat4_t mat4_make_rotation_x(float angle) {
    float c = cos(angle);
    float s = sin(angle);

    mat4_t rx = mat4_identity();
    rx.m[1][1] = c;
    rx.m[1][2] = -s;
    rx.m[2][1] = s;
    rx.m[2][2] = c;

    return rx;
}

mat4_t mat4_make_rotation_y(float angle) {
    float c = cos(angle);
    float s = sin(angle);

    mat4_t ry = mat4_identity();
    ry.m[0][0] = c;
    ry.m[0][2] = s;
    ry.m[2][0] = -s;
    ry.m[2][2] = c;

    return ry;
}

mat4_t mat4_make_rotation_z(float angle) {
    float c = cos(angle);
    float s = sin(angle);

    mat4_t rz = mat4_identity();
    rz.m[0][0] = c;
    rz.m[0][1] = -s;
    rz.m[1][0] = s;
    rz.m[1][1] = c;

    return rz;
}

mat4_t mat4_make_translation(float tx, float ty, float tz) {
    mat4_t t = mat4_identity();
    t.m[0][3] = tx;
    t.m[1][3] = ty;
    t.m[2][3] = tz;
    return t;
}

mat4_t mat4_make_look_at(vec3_t eye, vec3_t target, vec3_t up) {
    vec3_t z = vec3_sub(target, eye);
    vec3_normalize(&z);
    vec3_t x = vec3_cross(up, z);
    vec3_normalize(&x);

    // already normal
    vec3_t y = vec3_cross(z, x);

    mat4_t v = {
        {{x.x, x.y, x.z, -vec3_dot(x, eye)},
         {y.x, y.y, y.z, -vec3_dot(y, eye)},
         {z.x, z.y, z.z, -vec3_dot(z, eye)},
         {0.f, 0.f, 0.f, 1.f}},
    };

    return v;
}

mat4_t mat4_make_perspective(float fov, float inv_aspect, float znear, float zfar) {
    mat4_t p = {.m = {{0}}};

    p.m[0][0] = inv_aspect * (1.0f / tan(fov / 2.0f)); // x normalization
    p.m[1][1] = (1.0f / tan(fov / 2.0f));              // y normalization
    p.m[2][2] = zfar / (zfar - znear);                 // z normalization
    p.m[2][3] = (-zfar * znear) / (zfar - znear);      // z offset by znear
    p.m[3][2] = 1.0f;                                  // z stored in w, for perspective divide

    return p;
}

vec4_t mat4_mul_vec4(const mat4_t *m, vec4_t v) {
    vec4_t result = {
        .x = m->m[0][0] * v.x + m->m[0][1] * v.y + m->m[0][2] * v.z + m->m[0][3] * v.w,
        .y = m->m[1][0] * v.x + m->m[1][1] * v.y + m->m[1][2] * v.z + m->m[1][3] * v.w,
        .z = m->m[2][0] * v.x + m->m[2][1] * v.y + m->m[2][2] * v.z + m->m[2][3] * v.w,
        .w = m->m[3][0] * v.x + m->m[3][1] * v.y + m->m[3][2] * v.z + m->m[3][3] * v.w,
    };

    return result;
}

vec4_t mat4_mul_vec4_project(const mat4_t *p, vec4_t v) {
    vec4_t result = mat4_mul_vec4(p, v); // normal multiplication

    // Perspective divide by original z (stored in w), making sure to avoid div
    // by zero
    if (result.w != 0.0f) {
        result.x /= result.w;
        result.y /= result.w;
        result.z /= result.w;
    }

    return result;
}

// Easily vectorized when -O2
mat4_t mat4_mul_mat4(const mat4_t *a, const mat4_t *b) {
    mat4_t result;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            float dot = 0.0f;
            for (int i = 0; i < 4; i++) {
                dot += a->m[row][i] * b->m[i][col];
            }
            result.m[row][col] = dot;
        }
    }

    return result;
}
