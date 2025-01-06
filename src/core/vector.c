#include "vector.h"

#include <math.h>

float vec2_length(vec2_t v) { return sqrtf(v.x * v.x + v.y * v.y); }

vec2_t vec2_add(vec2_t v1, vec2_t v2) {
    vec2_t result = {
        v1.x + v2.x,
        v1.y + v2.y,
    };
    return result;
}
vec2_t vec2_sub(vec2_t v1, vec2_t v2) {
    vec2_t result = {
        v1.x - v2.x,
        v1.y - v2.y,
    };
    return result;
}

vec2_t vec2_mul(vec2_t v, float s) {
    vec2_t result = {
        v.x * s,
        v.y * s,
    };
    return result;
}

vec2_t vec2_div(vec2_t v, float s) {
    vec2_t result = {
        v.x / s,
        v.y / s,
    };
    return result;
}

float vec2_dot(vec2_t a, vec2_t b) { return a.x * b.x + a.y * b.y; }

void vec2_normalize(vec2_t *v) { *v = vec2_div(*v, vec2_length(*v)); }

float vec2_cross(vec2_t a, vec2_t b) { return a.x * b.y - a.y * b.x; }

float vec3_length(vec3_t v) { return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); }

vec3_t vec3_add(vec3_t v1, vec3_t v2) {
    vec3_t result = {
        v1.x + v2.x,
        v1.y + v2.y,
        v1.z + v2.z,
    };
    return result;
}

vec3_t vec3_sub(vec3_t v1, vec3_t v2) {
    vec3_t result = {
        v1.x - v2.x,
        v1.y - v2.y,
        v1.z - v2.z,
    };
    return result;
}

vec3_t vec3_mul(vec3_t v, float s) {
    vec3_t result = {
        v.x * s,
        v.y * s,
        v.z * s,
    };
    return result;
}

vec3_t vec3_div(vec3_t v, float s) {
    vec3_t result = {
        v.x / s,
        v.y / s,
        v.z / s,
    };
    return result;
}

vec3_t vec3_cross(vec3_t a, vec3_t b) {
    vec3_t result = {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x,
    };
    return result;
}

float vec3_dot(vec3_t a, vec3_t b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

void vec3_normalize(vec3_t *v) { *v = vec3_div(*v, vec3_length(*v)); }

vec3_t vec3_rotate_x(vec3_t v, float angle) {
    vec3_t result = {
        v.x,
        v.y * cosf(angle) - v.z * sinf(angle),
        v.y * sinf(angle) + v.z * cosf(angle),
    };
    return result;
}

vec3_t vec3_rotate_y(vec3_t v, float angle) {
    vec3_t result = {
        v.x * cosf(angle) - v.z * sinf(angle),
        v.y,
        v.x * sinf(angle) + v.z * cosf(angle),
    };
    return result;
}

vec3_t vec3_rotate_z(vec3_t v, float angle) {
    vec3_t result = {
        v.x * cosf(angle) - v.y * sinf(angle),
        v.x * sinf(angle) + v.y * cosf(angle),
        v.z,
    };
    return result;
}

vec4_t vec3_to_vec4(vec3_t v) {
    vec4_t result = {
        v.x,
        v.y,
        v.z,
        1.0f,
    };
    return result;
}

vec3_t vec4_to_vec3(vec4_t v) {
    vec3_t result = {
        v.x,
        v.y,
        v.z,
    };
    return result;
}

vec2_t vec4_to_vec2(vec4_t v) {
    vec2_t result = {
        v.x,
        v.y,
    };
    return result;
}
