#ifndef ASSET_LOAD_H
#define ASSET_LOAD_H

#include "core/pool.h"

enum ASS_Manager_Constants {
    ASS_INVALID_ITEM_ID = -1,
};

typedef struct ASS_Manager ASS_Manager;
struct ASS_Manager {
    Pool mesh_pool;
    Pool texture_pool;
};

void ass_manager_init(ASS_Manager *asset_manager);

// Returns a handle to a mesh, managed by asset manager
i64 ass_load_mesh_obj(ASS_Manager *asset_manager, char *file_name);

#endif // ASSET_LOAD_H
