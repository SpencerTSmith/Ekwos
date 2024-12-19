#ifndef PRIMITIVES_H
#define PRIMITIVES_H

#include <stdint.h>

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef float f32;
typedef double f64;

enum exit_code {
    EXT_SUCCESS,
    EXT_GLFW_WINDOW_CREATION,
    EXT_ARENA_ALLOCATION,
};

#endif // PRIMITIVES_H
