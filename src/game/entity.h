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
  u64 next_entity_id;
};

typedef u32 Entity_Flags;
enum Entity_Flags {
  EK_ENTITY_FLAG_DEFAULTS = 0,
};

typedef struct Entity Entity;
struct Entity {
  // Valid id's start at 1, if id is 0 = uninitialized
  u32 id;
  Entity_Flags flags;

  // Transforms
  vec3 scale;
  vec3 position;
  vec3 rotation;

  vec3 color;

  // TODO(ss): Handles into an asset manager, instead
  RND_Mesh *mesh;
};

enum Entity_Constants {
  ENTITY_INVALID_ID = 0,
  ENTITY_MAX_NUM = 10000,
};

Entity_Pool entity_pool_create(u64 capacity);
void entity_pool_free(Entity_Pool *pool);

Entity *entity_create(Entity_Pool *entity_pool, ASS_Manager *asset_manager, Entity_Flags flags,
                      vec3 position, vec3 rotation, vec3 scale, vec3 color, RND_Mesh *mesh);
void entity_free(Entity_Pool *entity_pool, Entity *entity);

mat4 entity_model_transform(Entity *entity);

#endif // ENTITY_H
