#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "core/pool.h"
#include "render/render_context.h"
#include "render/render_mesh.h"

enum ASS_Manager_Constants {
  ASS_INVALID_ITEM_ID = -1,
  ASS_MAX_ENTRIES = 64,
  ASS_MAX_MESHES = 32,
  ASS_MAX_TEXTURES = 32,
  ASS_MAX_FILE_NAME = 128,
};

typedef enum ASS_Type {
  ASS_TYPE_UNKOWN,
  ASS_TYPE_MESH,
  ASS_TYPE_TEXTURE,
  ASS_TYPE_COUNT,
} ASS_Type;

typedef struct ASS_Manager ASS_Manager;
struct ASS_Manager {
  Pool entry_pool;

  // Individual asset type pools
  Pool mesh_pool;
};

typedef struct ASS_Entry ASS_Entry;
struct ASS_Entry {
  u32 id;
  u32 reference_count;

  // Other way?
  char name[ASS_MAX_FILE_NAME];

  // Tagged union for different asset types
  ASS_Type type;
  union {
    RND_Mesh *mesh_data;
    // Textures, sounds, etc
  };

  void *next_in_hash;
};

// TODO(ss): Would be nice to use handles instead
typedef struct ASS_Handle ASS_Handle;
struct ASS_Handle {
  u32 id;
  ASS_Type type;
};

// TODO(ss): Could change pool implementation to allow using backing buffer... so we can keep
// the asset manager in the same arena? Or should they get their own indiviually allocated pools?
void ass_manager_init(Arena *arena, ASS_Manager *asset_manager);
void ass_manager_free(ASS_Manager *ass, RND_Context *rc);

// TODO(ss): perhaps unify entry creation... probably not too expensive to just check at runtime
// what file extension

// Returns a handle to a mesh, managed by asset manager, OJB loader taken
// straight from a previous project... needs work probably
ASS_Entry *ass_load_mesh_obj(ASS_Manager *asset_manager, RND_Context *render_context,
                             char *file_name);
ASS_Entry *ass_load_mesh_gtlf(ASS_Manager *asset_manager, RND_Context *render_context,
                              char *file_name);

void ass_free_entry(ASS_Manager *manager, RND_Context *render_context, ASS_Entry *asset_entry);

#endif // ASSET_MANAGER_H
