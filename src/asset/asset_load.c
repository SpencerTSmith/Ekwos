#include "asset/asset_load.h"

#include "core/arena.h"
#include "core/linear_algebra.h"
#include "core/log.h"
#include "core/thread_context.h"
#include "render/render_mesh.h"

#include <errno.h>
#include <stdio.h>

void ass_manager_init(Arena *arena, ASS_Manager *ass) {
    ass->mesh_pool = pool_create_type(ASS_MAX_MESHES, RND_Mesh);
    ass->mesh_reference_counts = arena_calloc(arena, ASS_MAX_MESHES, u32);
    ass->mesh_count = 0;
}

i32 ass_load_mesh_obj(ASS_Manager *ass, RND_Context *rc, char *file_name) {
    FILE *obj_file = fopen(file_name, "rb");

    if (obj_file == NULL) {
        LOG_ERROR("Failed to open .obj file \"%s\", (%s)", file_name, strerror(errno));
        return ASS_INVALID_ITEM_ID;
    }

    // HACK(ss): 2 Pass approach, so I can just use the scratch pad bump allocator and allocate
    // upfront

    char line[512];
    u32 vertex_count = 0;
    u32 index_count = 0;
    while (fgets(line, 512, obj_file) != NULL) {
        if (line[0] == '\0') // Empty
            continue;
        if (line[0] == '#') // Comment
            continue;

        // Real stuff
        if (line[0] == 'v' && line[1] == ' ') { // Vertices
            vertex_count++;
        } else if (line[0] == 'v' && line[1] == 't') { // Vertex UV's
            continue;
        } else if (line[0] == 'f' && line[1] == ' ') { // Face Indices
            index_count++;
        } // Potentially other things
    }
    rewind(obj_file);

    Scratch scratch = thread_get_scratch();

    RND_Vertex *vertices = arena_calloc(scratch.arena, vertex_count, RND_Vertex);
    u32 vertex_current_index = 0;

    u32 *indices = arena_calloc(scratch.arena, index_count * 3, u32);
    u32 index_current_index = 0;

    while (fgets(line, 512, obj_file) != NULL) {
        if (line[0] == '\0') // Empty
            continue;
        if (line[0] == '#') // Comment
            continue;

        // Real stuff
        if (line[0] == 'v' && line[1] == ' ') { // Vertices
            vec3 obj_vertex;
            if (!sscanf(line, "v %f %f %f", &obj_vertex.x, &obj_vertex.y, &obj_vertex.z)) {
                fprintf(stderr, "Error reading vertex data .obj file");
            }

            // HACK(ss): This will probably break in the future when we collect more than jus the
            // vertex position
            vertices[vertex_current_index++].position = obj_vertex;

        } else if (line[0] == 'v' && line[1] == 't') { // Vertex UV's
            // vec2 coord;
            // if (!sscanf(line, "vt %f %f", &coord.u, &coord.v)) {
            //     fprintf(stderr, "Error reading vertex texture coordinate data .obj file");
            // }
            // // adjust because .obj files have an inverted v compared to the renderer
            // coord.v = 1 - coord.v;
            // // array_push(texcoords, coord);

        } else if (line[0] == 'f' && line[1] == ' ') { // Face Indices
            u32 vertex_indices[3];
            u32 texture_indices[3];
            u32 normal_indices[3];

            if (!sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d ", &vertex_indices[0],
                        &texture_indices[0], &normal_indices[0], &vertex_indices[1],
                        &texture_indices[1], &normal_indices[1], &vertex_indices[2],
                        &texture_indices[2], &normal_indices[2])) {
                fprintf(stderr, "Error reading vertex indice data from .obj file");
            }

            indices[index_current_index++] = vertex_indices[0];
            indices[index_current_index++] = vertex_indices[1];
            indices[index_current_index++] = vertex_indices[2];
        }
    }

    RND_Mesh *mesh = pool_alloc(&ass->mesh_pool);
    rnd_mesh_init(rc, mesh, vertices, vertex_count, indices, index_count);

    // And done with those vertices on the CPU side
    thread_end_scratch(&scratch);

    fclose(obj_file);
    return 0;
}
