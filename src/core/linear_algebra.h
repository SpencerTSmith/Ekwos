#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H
#include <math.h>

#include "core/common.h"

#define PI 3.14159265358979323846
/*
    Obviously huge inspiration from HandmadeMath
    Vectors are column vectors, Matrices are column major
    We assume a 0 - > 1 NDC and a right handed system, with z pointing inwards
*/

// TODO(ss): I've been looking through godbolt, and mat4_mul_vec4 seems to easily autovectorise in
// case -O3 and -mavx, we may not need to so explicit SIMD intrinsics but something to look into as
// we progress

typedef union vec2 vec2;
union vec2 {
    struct {
        f32 x, y;
    };
    struct {
        f32 u, v;
    };
    struct {
        f32 w, h;
    };
    f32 elements[2];
};

typedef union vec3 vec3;
union vec3 {
    struct {
        f32 x, y, z;
    };
    struct {
        f32 r, g, b;
    };
    struct {
        f32 _ignored_x;
        vec2 yz;
    };
    struct {
        vec2 xy;
        f32 _ignored_z;
    };
    struct {
        f32 _ignored_u;
        vec2 vw;
    };
    struct {
        vec2 uv;
        f32 _ignored_w;
    };
    f32 elements[3];
};

typedef union vec4 vec4;
union vec4 {
    struct {
        union {
            struct {
                f32 x, y, z;
            };
            vec3 xyz;
        };
        f32 w;
    };
    struct {
        union {
            struct {
                f32 r, g, b;
            };
            vec3 rgb;
        };
        f32 a;
    };
    struct {
        vec2 xy;
        vec2 _ignored_zw;
    };
    struct {
        f32 _ignored_x;
        vec2 yz;
        f32 _ignored_w;
    };
    struct {
        vec2 _ignored_xy;
        vec2 zw;
    };
    f32 elements[4];
};

typedef union mat4 mat4;
union mat4 {
    f32 m[4][4];
    vec4 cols[4];
};

#define vec2(x, y) vec2_make(x, y)
static inline vec2 vec2_make(f32 x, f32 y) {
    vec2 result;
    result.x = x;
    result.y = y;

    return result;
}

static inline f32 vec2_length(vec2 v) { return sqrtf(v.x * v.x + v.y * v.y); }

static inline vec2 vec2_add(vec2 v1, vec2 v2) {
    vec2 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;

    return result;
}
static inline vec2 vec2_sub(vec2 v1, vec2 v2) {
    vec2 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;

    return result;
}

static inline vec2 vec2_mul(vec2 v, f32 s) {
    vec2 result;
    result.x = v.x * s;
    result.y = v.y * s;

    return result;
}

static inline vec2 vec2_div(vec2 v, f32 s) {
    vec2 result;
    result.x = v.x / s;
    result.y = v.y / s;

    return result;
}

static inline f32 vec2_dot(vec2 a, vec2 b) { return a.x * b.x + a.y * b.y; }

// Only 1 division like this
static inline vec2 vec2_norm(vec2 v) { return vec2_mul(v, 1 / vec2_length(v)); }

static inline f32 vec2_cross(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }

#define vec3(x, y, z) vec3_make(x, y, z)
static inline vec3 vec3_make(f32 x, f32 y, f32 z) {
    vec3 result;
    result.x = x;
    result.y = y;
    result.z = z;

    return result;
}

static inline f32 vec3_len(vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

static inline vec3 vec3_add(vec3 v1, vec3 v2) {
    vec3 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;

    return result;
}

static inline vec3 vec3_sub(vec3 v1, vec3 v2) {
    vec3 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;

    return result;
}

static inline vec3 vec3_mul(vec3 v, f32 s) {
    vec3 result;
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;

    return result;
}

static inline vec3 vec3_div(vec3 v, f32 s) {
    vec3 result;
    result.x = v.x / s;
    result.y = v.y / s;
    result.z = v.z / s;

    return result;
}

static inline vec3 vec3_cross(vec3 left, vec3 right) {
    vec3 result;
    result.x = (left.y * right.z) - (left.z * right.y);
    result.y = (left.z * right.x) - (left.x * right.z);
    result.z = (left.x * right.y) - (left.y * right.x);

    return result;
}

static inline f32 vec3_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

static inline vec3 vec3_norm(vec3 v) { return vec3_mul(v, 1.0f / vec3_len(v)); }

static inline vec3 vec3_rotate_x(vec3 v, f32 angle) {
    vec3 result;
    result.x = v.x;
    result.y = v.y * cosf(angle) - v.z * sinf(angle);
    result.z = v.y * sinf(angle) + v.z * cosf(angle);

    return result;
}

static inline vec3 vec3_rotate_y(vec3 v, f32 angle) {
    vec3 result;
    result.x = v.x * cosf(angle) - v.z * sinf(angle);
    result.y = v.y;
    result.z = v.x * sinf(angle) + v.z * cosf(angle);

    return result;
}

static inline vec3 vec3_rotate_z(vec3 v, f32 angle) {
    vec3 result;
    result.x = v.x * cosf(angle) - v.y * sinf(angle);
    result.y = v.x * sinf(angle) + v.y * cosf(angle);
    result.z = v.z;

    return result;
}

static inline vec4 vec3_to_vec4(vec3 v) {
    vec4 result;
    result.xyz = v;
    result.w = 1.0f;

    return result;
}

#define vec4(x, y, z, w) vec4_make(x, y, z, w)
static inline vec4 vec4_make(f32 x, f32 y, f32 z, f32 w) {
    vec4 result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;

    return result;
}

static inline f32 vec4_len(vec4 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w); }

static inline vec4 vec4_add(vec4 v1, vec4 v2) {
    vec4 result;
    result.x = v1.x + v2.x;
    result.y = v1.y + v2.y;
    result.z = v1.z + v2.z;
    result.w = v1.w + v2.w;

    return result;
}

static inline vec4 vec4_sub(vec4 v1, vec4 v2) {
    vec4 result;
    result.x = v1.x - v2.x;
    result.y = v1.y - v2.y;
    result.z = v1.z - v2.z;
    result.w = v1.w - v2.w;

    return result;
}

static inline vec4 vec4_mul(vec4 v, f32 s) {
    vec4 result;
    result.x = v.x * s;
    result.y = v.y * s;
    result.z = v.z * s;
    result.w = v.w * s;

    return result;
}

static inline vec4 vec4_div(vec4 v, f32 s) {
    vec4 result;
    result.x = v.x / s;
    result.y = v.y / s;
    result.z = v.z / s;
    result.w = v.w / s;

    return result;
}

static inline f32 vec4_dot(vec4 a, vec4 b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

static inline vec4 vec4_norm(vec4 v) { return vec4_mul(v, 1.0f / vec4_len(v)); }

#define mat4_identity() mat4_diagonal(1.0f)
static inline mat4 mat4_diagonal(f32 d) {
    mat4 m = {0};
    m.cols[0].x = d;
    m.cols[1].y = d;
    m.cols[2].z = d;
    m.cols[3].w = d;

    return m;
}

static inline mat4 mat4_scale(vec3 v) {
    mat4 s = mat4_identity();
    s.cols[0].x = v.x;
    s.cols[1].y = v.y;
    s.cols[2].z = v.z;

    return s;
}

#define mat4_make_rotation_x(radians) mat4_rotation(radians, vec3(1.0f, 0.0f, 0.0f))
#define mat4_make_rotation_y(radians) mat4_rotation(radians, vec3(0.0f, 1.0f, 0.0f))
#define mat4_make_rotation_z(radians) mat4_rotation(radians, vec3(0.0f, 0.0f, 1.0f))

// General form taken from wikipedia
static inline mat4 mat4_rotation(f32 radians, vec3 axis) {
    axis = vec3_norm(axis);
    f32 sin = sinf(radians);
    f32 cos = cosf(radians);
    f32 one_minus_cos = 1.0f - cos;

    mat4 r = mat4_identity();
    r.cols[0].x = (axis.x * axis.x * one_minus_cos) + cos;
    r.cols[0].y = (axis.x * axis.y * one_minus_cos) + (axis.z * sin);
    r.cols[0].z = (axis.x * axis.z * one_minus_cos) - (axis.y * sin);

    r.cols[1].x = (axis.y * axis.x * one_minus_cos) - (axis.z * sin);
    r.cols[1].y = (axis.y * axis.y * one_minus_cos) + cos;
    r.cols[1].z = (axis.y * axis.z * one_minus_cos) + (axis.x * sin);

    r.cols[2].x = (axis.z * axis.x * one_minus_cos) + (axis.y * sin);
    r.cols[2].y = (axis.z * axis.y * one_minus_cos) - (axis.x * sin);
    r.cols[2].z = (axis.z * axis.z * one_minus_cos) + cos;

    return r;
}

// static inline mat4 mat4_rotation_tait_zxy(vec3 rotations) {
//
// }

static inline mat4 mat4_translation(vec3 v) {
    mat4 t = mat4_identity();
    t.cols[3].x = v.x;
    t.cols[3].y = v.y;
    t.cols[3].z = v.z;

    return t;
}

static inline mat4 mat4_look_at(vec3 eye, vec3 target, vec3 up) {
    vec3 z = vec3_sub(target, eye);
    z = vec3_norm(z);
    vec3 x = vec3_cross(z, up);
    x = vec3_norm(x);

    // already normal
    vec3 y = vec3_cross(z, x);

    f32 x_dot = -vec3_dot(x, eye);
    f32 y_dot = -vec3_dot(y, eye);
    f32 z_dot = -vec3_dot(z, eye);

    mat4 m = {0};
    m.cols[0] = vec4(x.x, y.x, z.x, 0.0f);
    m.cols[1] = vec4(x.y, y.y, z.y, 0.0f);
    m.cols[2] = vec4(x.z, y.z, z.z, 0.0f);
    m.cols[3] = vec4(x_dot, y_dot, z_dot, 1.0f);

    return m;
}

static inline mat4 mat4_perspective(f32 fov, f32 aspect_ratio, f32 z_near, f32 z_far) {
    mat4 p = {0};
    f32 cotangent = 1.0f / tan(fov / 2.0f);
    p.m[0][0] = cotangent / aspect_ratio;            // x normalization
    p.m[1][1] = cotangent;                           // y normalization
    p.m[2][2] = z_far / (z_near - z_far);            // z normalization
    p.m[3][2] = (z_far * z_near) / (z_near - z_far); // z offset by znear
    p.m[2][3] = -1.0f;                               // z stored in w, for perspective divide

    return p;
}

static inline vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    vec4 result;
    result.x = m.cols[0].x * v.x;
    result.y = m.cols[0].y * v.x;
    result.z = m.cols[0].z * v.x;
    result.w = m.cols[0].w * v.x;

    result.x += m.cols[1].x * v.y;
    result.y += m.cols[1].y * v.y;
    result.z += m.cols[1].z * v.y;
    result.w += m.cols[1].w * v.y;

    result.x += m.cols[2].x * v.z;
    result.y += m.cols[2].y * v.z;
    result.z += m.cols[2].z * v.z;
    result.w += m.cols[2].w * v.z;

    result.x += m.cols[3].x * v.w;
    result.y += m.cols[3].y * v.w;
    result.z += m.cols[3].z * v.w;
    result.w += m.cols[3].w * v.w;

    return result;
}

static inline mat4 mat4_mul(mat4 left, mat4 right) {
    mat4 result;
    result.cols[0] = mat4_mul_vec4(left, right.cols[0]);
    result.cols[1] = mat4_mul_vec4(left, right.cols[1]);
    result.cols[2] = mat4_mul_vec4(left, right.cols[2]);
    result.cols[3] = mat4_mul_vec4(left, right.cols[3]);

    return result;
}
#endif // LINEAR_ALGEBRA_H
