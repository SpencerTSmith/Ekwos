#ifndef ENTITY_H
#define ENTITY_H

#include "asset/asset_manager.h"
#include "core/common.h"
#include "core/linear_algebra.h"
#include "core/pool.h"

#include "render/render_mesh.h"

typedef struct Entity_Pool Entity_Pool;
struct Entity_Pool {
  Pool pool;
  i32 next_entity_id;
};

// In future we might want 64 bits for flags
typedef enum Entity_Flags {
  ENTITY_FLAG_INVALID = -1,
  ENTITY_FLAG_DEFAULT = 0,
} Entity_Flags;

typedef struct Entity Entity;
struct Entity {
  u64 id;
  // HACK(ss): Storing this down here because pool allocator just stores the free list directly in
  // the buffer meaning that this needs to not be overwritten by the node pointer, this is
  // absolutely terrible design, but it works
  Entity_Flags flags;

  // Transforms
  vec3 scale;
  vec3 position;
  vec3 rotation;

  vec3 color;

  ASS_Entry *mesh_asset;
};

enum Entity_Constants {
  ENTITY_MAX_NUM = 1000,
};

Entity_Pool entity_pool_make(u64 capacity);
void entity_pool_free(Entity_Pool *pool);

Entity *entity_make(Entity_Pool *entity_pool, RND_Context *render_context,
                    ASS_Manager *asset_manager, Entity_Flags flags, vec3 position, vec3 rotation,
                    vec3 scale, vec3 color, char *mesh_file);
void entity_free(Entity_Pool *entity_pool, Entity *entity);

mat4 entity_model_transform(const Entity *entity);
// Inverse transpose of the model transform
mat4 entity_normal_matrix(const Entity *entity);

#endif // ENTITY_H
