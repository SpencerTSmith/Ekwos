#include <stdio.h>
#include <stdlib.h>

#include "primitives.h"

struct shader_data {
    u8 *data;
    u64 size;
};

void read_shader_file(const char &file_path, shader_data &shader_data);
