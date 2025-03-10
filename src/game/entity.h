#ifndef ENTITY_H
#define ENTITY_H

#include "asset/asset_manager.h"
#include "core/common.h"
#include "core/linear_algebra.h"
#include "core/pool.h"

#include "render/render_mesh.h"

typedef u64 Entity_ID;

enum Entity_Constants {
  ENTITY_MAX_NUM = 1000,
  ENTITY_ID_OFFSET = -1,
};

typedef struct Entity_Pool Entity_Pool;
struct Entity_Pool {
  Pool pool;
  Entity_ID next_entity_id;
};

// In future we might want 64 bits for flags
typedef enum Entity_Flags {
  ENTITY_FLAG_INVALID = -1,
  ENTITY_FLAG_DEFAULT = 0,
} Entity_Flags;

typedef struct Entity Entity;
struct Entity {
  Entity_ID id; // As stated below this acts as padding as well for when a pointer is held here
                // instead, hope that 128 bit doesn't come any time soon

  // HACK(ss): Storing this down here because pool allocator just stores the free list directly in
  // the buffer meaning that this needs to not be overwritten by the node pointer, this might be
  // terrible(genius) design, but it works
  Entity_Flags flags;

  // Transforms
  vec3 scale;
  vec3 position;
  vec3 rotation;

  ASS_Entry *mesh_asset;
};

#if 0
typedef enum Entity_Slot_Type {
  ENTITY_SLOT_FREED = 0,
  ENTITY_SLOT_OCCUPIED = 1,
} Entity_Slot_Type;

typedef struct Entity_Free Entity_Free;
struct Entity_Free {
  Entity_Free *next;
};

// Easy little union to determine if this spot is occupied or not
typedef struct Entity_Slot Entity_Slot;
struct Entity_Slot {
  Entity_Slot_Type type;
  union {
    Entity_Free *free;
    Entity entity;
  };
};

typedef struct Entity_Pool Entity_Pool;
struct Entity_Pool {
  Pool pool;
  Entity_ID next_entity_id;
	Entity_Free *free;
};
#endif

Entity_Pool entity_pool_make(u64 capacity);
void entity_pool_free(Entity_Pool *pool);

Entity *entity_make(Entity_Pool *ep, RND_Context *rc, ASS_Manager *am, Entity_Flags flags,
                    vec3 position, vec3 rotation, vec3 scale, char *mesh_file);
void entity_free(Entity_Pool *entity_pool, RND_Context *render_context, ASS_Manager *asset_manager,
                 Entity *entity);

Entity *entity_get(Entity_Pool *ep, Entity_ID id);

mat4 entity_model_mat4(const Entity *entity);
// Inverse transpose of the model transform
mat4 entity_normal_mat4(const Entity *entity);

#endif // ENTITY_H
