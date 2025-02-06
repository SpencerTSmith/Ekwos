#include "asset/asset_load.h"

#include "core/linear_algebra.h"
#include "core/log.h"

#include <errno.h>
#include <stdio.h>

i64 ass_load_mesh_obj(ASS_Manager *ass, char *file_name) {
    FILE *obj_file = fopen(file_name, "rb");

    if (obj_file == NULL) {
        LOG_ERROR("Failed to open .obj file \"%s\", (%s)", file_name, strerror(errno));
        return ASS_INVALID_ITEM_ID;
    }

    char line[512];

    // HACK(ss): 2 Pass approach, so I can just use the scratch pad bump allocator and allocate
    // upfront

    vec2 *texcoords = NULL; // dynamic array

    while (fgets(line, 512, obj_file)) {
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

            // array_push(mesh->vertices, obj_vertex);
        } else if (line[0] == 'v' && line[1] == 't') { // Vertex UV's
            vec2 coord;
            if (!sscanf(line, "vt %f %f", &coord.u, &coord.v)) {
                fprintf(stderr, "Error reading vertex texture coordinate data .obj file");
            }
            // adjust because .obj files have an inverted v compared to the renderer
            coord.v = 1 - coord.v;
            // array_push(texcoords, coord);
        } else if (line[0] == 'f' && line[1] == ' ') { // Face Indices
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];

            // Temporary fix for models without normals
            /*sscanf(line, "f %d/%d %d/%d %d/%d ",
                &vertex_indices[0], &texture_indices[0],
                &vertex_indices[1], &texture_indices[1],
                &vertex_indices[2], &texture_indices[2]
                );*/

            if (!sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d ", &vertex_indices[0],
                        &texture_indices[0], &normal_indices[0], &vertex_indices[1],
                        &texture_indices[1], &normal_indices[1], &vertex_indices[2],
                        &texture_indices[2], &normal_indices[2])) {
                fprintf(stderr, "Error reading vertex indice data from .obj file");
            }

            // face_t obj_face = {
            //     .a = vertex_indices[0] - 1, // adjust by 1, indices start at 1 in .obj format
            //     .b = vertex_indices[1] - 1,
            //     .c = vertex_indices[2] - 1,
            //     .a_uv = texcoords[texture_indices[0] - 1],
            //     .b_uv = texcoords[texture_indices[1] - 1],
            //     .c_uv = texcoords[texture_indices[2] - 1],
            //     .color = WHITE,
            // };
            // array_push(mesh->faces, obj_face);
        }
    }
    // array_free(texcoords);

    fclose(obj_file);

    return ASS_INVALID_ITEM_ID;
}
