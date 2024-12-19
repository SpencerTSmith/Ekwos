#include "shader.h"

#include <stdalign.h>

ShaderData read_shader_file(Arena *arena, const char *file_path) {
    ShaderData shader_data = {0};

    FILE *shader_file = fopen(file_path, "rb");
    if (shader_file == NULL) {
        fprintf(stderr, "Error: Failed to read shader file: %s\n", file_path);
        return shader_data;
    }

    if (fseek(shader_file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error: Failed to seek end of file: %s\n", file_path);
        fclose(shader_file);
        return shader_data;
    }

    u64 size = ftell(shader_file);
    if (size == -1L) {
        fprintf(stderr, "Error: Failed to get size of file: %s\n", file_path);
        fclose(shader_file);
        return shader_data;
    }

    shader_data.size = size;
    rewind(shader_file);

    shader_data.data = arena_alloc(arena, size, alignof(*shader_data.data));

    u64 bytes_read = fread(shader_data.data, 1, size, shader_file);
    if (bytes_read != size) {
        fprintf(stderr, "Error: shader bytes read into buffer does not match file size: %s\n",
                file_path);
        arena_pop(arena, size);
        fclose(shader_file);
        return shader_data;
    }

    fclose(shader_file);
    return shader_data;
}
