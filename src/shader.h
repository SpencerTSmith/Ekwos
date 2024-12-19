#ifndef SHADER_H
#define SHADER_H

#include <stdio.h>
#include <stdlib.h>

#include "arena.h"
#include "common.h"

typedef struct ShaderData ShaderData;
struct ShaderData {
    u8 *data;
    u64 size;
};

ShaderData read_shader_file(Arena *arena, const char *file_path);

#endif // SHADER_H
