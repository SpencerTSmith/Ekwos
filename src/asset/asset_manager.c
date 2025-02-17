#include "asset/asset_manager.h"

#include "core/arena.h"
#include "core/linear_algebra.h"
#include "core/log.h"
#include "core/thread_context.h"
#include "render/render_mesh.h"

#include <errno.h>
#include <stdio.h>

void ass_manager_init(Arena *arena, ASS_Manager *ass) {
  ass->entry_pool = pool_create_type(ASS_MAX_ENTRIES, ASS_Entry);
  ass->mesh_pool = pool_create_type(ASS_MAX_MESHES, RND_Mesh);
}

void *ass_load_mesh_obj(ASS_Manager *ass, RND_Context *rc, char *file_name) {
  FILE *obj_file = fopen(file_name, "rb");

  if (obj_file == NULL) {
    LOG_ERROR("Failed to open .obj file \"%s\", (%s)", file_name, strerror(errno));
    return NULL;
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
      index_count += 3;
    }
    // Potentially other things
  }
  rewind(obj_file);

  Scratch scratch = thread_get_scratch();

  RND_Vertex *vertices = arena_calloc(scratch.arena, vertex_count, RND_Vertex);
  u32 vertex_current_index = 0;

  u32 *indices = arena_calloc(scratch.arena, index_count, u32);
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
        LOG_ERROR("Error reading vertex data from .obj file (%s)", file_name);
      }

      vertices[vertex_current_index].position = obj_vertex;
      vec3_print(vertices[vertex_current_index].position);
      vertex_current_index++;

    } else if (line[0] == 'v' && line[1] == 't') { // Vertex UV's

    } else if (line[0] == 'f' && line[1] == ' ') { // Face Indices
      u32 vertex_indices[3];
      u32 texture_indices[3];
      u32 normal_indices[3];

      if (!sscanf(line, "f %u/%u/%u %u/%u/%u %u/%u/%u ", &vertex_indices[0], &texture_indices[0],
                  &normal_indices[0], &vertex_indices[1], &texture_indices[1], &normal_indices[1],
                  &vertex_indices[2], &texture_indices[2], &normal_indices[2])) {
        LOG_ERROR("Error reading vertex indice data from .obj file (%s)", file_name);
      }

      // Adjusting for the fact .obj indices start at 1
      indices[index_current_index + 0] = vertex_indices[0] - 1;
      indices[index_current_index + 1] = vertex_indices[1] - 1;
      indices[index_current_index + 2] = vertex_indices[2] - 1;
      LOG_DEBUG("Loaded index %u, %u, %u", indices[index_current_index + 0],
                indices[index_current_index + 1], indices[index_current_index + 2]);
      index_current_index += 3;
    }
  }

  // Get a new mesh out of the mesh pool, and initialize
  RND_Mesh *mesh = pool_alloc(&ass->mesh_pool);
  rnd_mesh_init(rc, mesh, vertices, vertex_count, indices, index_count);

  // And done with those vertices on the CPU side
  thread_end_scratch(&scratch);

  // Create a new asset entry
  ASS_Entry *entry = pool_alloc(&ass->entry_pool);
  entry->data = mesh;
  entry->reference_count++;
  entry->type = ASS_TYPE_MESH;
  entry->handle = 0;
  strcpy(entry->file_name, file_name);
  LOG_DEBUG("Loaded asset: %s", file_name);

  fclose(obj_file);
  return mesh;
}
