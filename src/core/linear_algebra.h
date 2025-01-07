#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H
#include <math.h>

#include "core/common.h"

// Obviously huge inspiration from HandmadeMath

// TODO(ss): SIMDify explicitly, don't leave it up to compiler
// And look into inlining everything

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
    vec4 rows[4];
};

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
static inline vec2 vec2_normalize(vec2 v) { return vec2_mul(v, 1 / vec2_length(v)); }

static inline f32 vec2_cross(vec2 a, vec2 b) { return a.x * b.y - a.y * b.x; }

static inline f32 vec3_length(vec3 v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

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

static inline vec3 vec3_cross(vec3 a, vec3 b) {
    vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

static inline f32 vec3_dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

static inline vec3 vec3_normalize(vec3 v) { return vec3_mul(v, 1 / vec3_length(v)); }

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

static inline f32 vec4_length(vec4 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

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

static inline vec4 vec4_normalize(vec4 v) { return vec4_mul(v, 1 / vec4_length(v)); }

static inline mat4 mat4_identity(void) {
    mat4 i = {.m = {
                  {1, 0, 0, 0},
                  {0, 1, 0, 0},
                  {0, 0, 1, 0},
                  {0, 0, 0, 1},
              }};
    return i;
}

static inline mat4 mat4_make_scale(f32 sx, f32 sy, f32 sz) {
    mat4 s = mat4_identity();
    s.m[0][0] = sx;
    s.m[1][1] = sy;
    s.m[2][2] = sz;

    return s;
}

static inline mat4 mat4_make_rotation_x(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    mat4 rx = mat4_identity();
    rx.m[1][1] = c;
    rx.m[1][2] = -s;
    rx.m[2][1] = s;
    rx.m[2][2] = c;

    return rx;
}

static inline mat4 mat4_make_rotation_y(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    mat4 ry = mat4_identity();
    ry.m[0][0] = c;
    ry.m[0][2] = s;
    ry.m[2][0] = -s;
    ry.m[2][2] = c;

    return ry;
}

static inline mat4 mat4_make_rotation_z(f32 angle) {
    f32 c = cosf(angle);
    f32 s = sinf(angle);

    mat4 rz = mat4_identity();
    rz.m[0][0] = c;
    rz.m[0][1] = -s;
    rz.m[1][0] = s;
    rz.m[1][1] = c;

    return rz;
}

static inline mat4 mat4_make_translation(f32 tx, f32 ty, f32 tz) {
    mat4 t = mat4_identity();
    t.m[0][3] = tx;
    t.m[1][3] = ty;
    t.m[2][3] = tz;
    return t;
}

static inline mat4 mat4_make_look_at(vec3 eye, vec3 target, vec3 up) {
    vec3 z = vec3_sub(target, eye);
    z = vec3_normalize(z);
    vec3 x = vec3_cross(up, z);
    x = vec3_normalize(x);

    // already normal
    vec3 y = vec3_cross(z, x);

    mat4 m = {
        {
            {x.x, x.y, x.z, -vec3_dot(x, eye)},
            {y.x, y.y, y.z, -vec3_dot(y, eye)},
            {z.x, z.y, z.z, -vec3_dot(z, eye)},
            {0.f, 0.f, 0.f, 1.f},
        },
    };

    return m;
}

static inline mat4 mat4_make_perspective(f32 fov, f32 inv_aspect, f32 znear, f32 zfar) {
    mat4 p = {0};

    p.m[0][0] = inv_aspect * (1.0f / tan(fov / 2.0f)); // x normalization
    p.m[1][1] = (1.0f / tan(fov / 2.0f));              // y normalization
    p.m[2][2] = zfar / (zfar - znear);                 // z normalization
    p.m[2][3] = (-zfar * znear) / (zfar - znear);      // z offset by znear
    p.m[3][2] = 1.0f;                                  // z stored in w, for perspective divide

    return p;
}

static inline vec4 mat4_mul_vec4(mat4 m, vec4 v) {
    vec4 result = {
        .x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w,
        .y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w,
        .z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w,
        .w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w,
    };

    return result;
}

// Easily vectorized when -O2
static inline mat4 mat4_mul_mat4(mat4 a, mat4 b) {
    mat4 result;

    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            f32 dot = 0.0f;
            for (int i = 0; i < 4; i++) {
                dot += a.m[row][i] * b.m[i][col];
            }
            result.m[row][col] = dot;
        }
    }

    return result;
}
#endif // LINEAR_ALGEBRA_H
