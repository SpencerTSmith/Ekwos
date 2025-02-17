#ifndef ASSET_MANAGER_H
#define ASSET_MANAGER_H

#include "core/pool.h"
#include "render/render_context.h"

enum ASS_Manager_Constants {
  ASS_INVALID_ITEM_ID = INT32_MAX,
  ASS_MAX_ENTRIES = 40,
  ASS_MAX_MESHES = 20,
  ASS_MAX_TEXTURES = 20,
};

typedef u32 ASS_Type;
enum ASS_Type {
  ASS_TYPE_MESH,
  ASS_TYPE_TEXTURE,
};

typedef struct ASS_Manager ASS_Manager;
struct ASS_Manager {
  Pool entry_pool;

  // Individual asset type pools
  Pool mesh_pool;
};

typedef struct ASS_Entry ASS_Entry;
struct ASS_Entry {
  u32 handle;
  u32 reference_count;

  char file_name[128];

  ASS_Type type;
  void *data;
};

// TODO(ss): Will need to change pool implementation to allow using backing buffer... so we can keep
// the asset manager in the same arena? Or should they get their own indiviually allocated pools?
void ass_manager_init(Arena *arena, ASS_Manager *asset_manager);

void ass_create_entry(ASS_Manager *asset_manager);

// Returns a handle to a mesh, managed by asset manager, taken straight from a previous
// project... needs work probably
void *ass_load_mesh_obj(ASS_Manager *asset_manager, RND_Context *render_context, char *file_name);

#endif // ASSET_MANAGER_H
