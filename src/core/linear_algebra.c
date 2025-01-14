#include "core/linear_algebra.h"
#include "core/log.h"

// bunch of little helpers for debugging, when stepping through with a debugger is inconvenient

void vec2_print(vec2 v) { LOG_DEBUG("vec2(%f, %f)", v.x, v.y); }

void vec3_print(vec3 v) { LOG_DEBUG("vec3(%f, %f, %f)", v.x, v.y, v.z); }

void vec4_print(vec4 v) { LOG_DEBUG("vec4(%f, %f, %f, %f)", v.x, v.y, v.z, v.w); }

void mat4_print(mat4 m) {
    LOG_DEBUG("mat4(%f, %f, %f, %f\n                    %f, %f, %f, %f\n                    %f, "
              "%f, %f, %f\n                    %f, %f, %f, %f)",
              m.cols[0].x, m.cols[0].y, m.cols[0].z, m.cols[0].w, m.cols[1].x, m.cols[1].y,
              m.cols[1].z, m.cols[1].w, m.cols[2].x, m.cols[2].y, m.cols[2].z, m.cols[2].w,
              m.cols[3].x, m.cols[3].y, m.cols[3].z, m.cols[3].w);
}
