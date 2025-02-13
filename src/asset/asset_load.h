#ifndef ASSET_LOAD_H
#define ASSET_LOAD_H

#include "core/pool.h"
#include "render/render_context.h"

// TODO(ss): Really need to think about this... what's a nice way to do this?
enum ASS_Manager_Constants {
    ASS_INVALID_ITEM_ID = -1,
    ASS_MAX_MESHES = 20,
    ASS_MAX_TEXTURES = 20,
};

typedef struct ASS_Manager ASS_Manager;
struct ASS_Manager {
    Pool mesh_pool;
    u32 *mesh_reference_counts;
    u32 mesh_count;
};

typedef struct ASS_Entry ASS_Entry;
struct ASS_Entry {
    char file_name[256];
    u32 id;
    u32 reference_count;
    void *data;
};

void ass_manager_init(Arena *arena, ASS_Manager *asset_manager);

// Returns a handle to a mesh, managed by asset manager, taken straight from a previous project...
// needs work probably
i32 ass_load_mesh_obj(ASS_Manager *asset_manager, RND_Context *render_context, char *file_name);

#endif // ASSET_LOAD_H
