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

void ass_manager_free(ASS_Manager *ass, RND_Context *rc) {
  u32 mesh_count = 0;
  RND_Mesh *meshes = pool_as_array(&ass->mesh_pool, &mesh_count);
  for (u32 i = 0; i < mesh_count; i++) {
    rnd_mesh_free(rc, &meshes[i]);
  }
  pool_free(&ass->entry_pool);

  pool_free(&ass->mesh_pool);
}

// Check if we've already loaded this file
ASS_Entry *ass_find_existing(ASS_Manager *ass, char *name) {
  // TODO(ss): This is not going to scale, probably need hash table
  u32 entries_count = 0;
  ASS_Entry *entries = pool_as_array(&ass->entry_pool, &entries_count);
  for (u32 i = 0; i < entries_count; i++) {
    if (strcmp(entries[i].name, name) == 0) {
      return &entries[i];
    }
  }

  return NULL;
}

ASS_Entry ass_load_mesh_obj(ASS_Manager *ass, RND_Context *rc, char *file_name) {
  // Check if we already loaded this
  ASS_Entry *existing = ass_find_existing(ass, file_name);
  if (existing != NULL) {
    existing->reference_count++;
    LOG_DEBUG("Asset (%s) has been reused: reference count = %u", existing->name,
              existing->reference_count);
    return *existing;
  }

  FILE *obj_file = fopen(file_name, "rb");

  if (obj_file == NULL) {
    LOG_ERROR("Failed to open .obj file \"%s\", (%s)... loading default cube", file_name,
              strerror(errno));

    // Check if we've already loaded the default cube
    ASS_Entry *loaded_cube = ass_find_existing(ass, "default_cube");
    if (loaded_cube != NULL) {
      loaded_cube->reference_count++;
      LOG_DEBUG("Asset (%s) has been reused: reference count = %u", loaded_cube->name,
                loaded_cube->reference_count);
      return *loaded_cube;
    }

    // First time an invalid file was loaded, load the default cube into memory
    RND_Mesh *mesh = pool_alloc(&ass->mesh_pool);
    rnd_mesh_default_cube(rc, mesh);
    ASS_Entry *entry = pool_alloc(&ass->entry_pool);
    entry->data = mesh;
    entry->reference_count++;
    entry->type = ASS_TYPE_MESH;
    entry->id = 0;
    strcpy(entry->name, "default_cube");

    return *entry;
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

    // Real stuff, assuming same # of vertices as texture coords, etc
    if (line[0] == 'v' && line[1] == ' ') { // Vertices
      vertex_count++;
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
      vertices[vertex_current_index].color = vec3(1.0f, 0.5f, 0.2f); // Default
      vertex_current_index++;

    } else if (line[0] == 'v' && line[1] == 't') { // Vertex UV's

    } else if (line[0] == 'v' && line[1] == 'n') {

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
  entry->id = 0;
  strcpy(entry->name, file_name);
  LOG_DEBUG("Loaded asset (%s) for the first time", file_name);

  fclose(obj_file);
  return *entry;
}
