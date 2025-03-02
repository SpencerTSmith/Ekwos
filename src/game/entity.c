#include "game/entity.h"

Entity_Pool entity_pool_make(u64 capacity) {
  ASSERT(capacity <= ENTITY_MAX_NUM, "Entity pool created with capcity greater than max");
  Entity_Pool pool = {
      .pool = pool_make_type(capacity, Entity),
      .next_entity_id = 1,
  };

  return pool;
}

void entity_pool_free(Entity_Pool *pool) {
  pool_free(&pool->pool);
  ZERO_STRUCT(pool);
}

Entity *entity_make(Entity_Pool *ep, RND_Context *rc, ASS_Manager *am, Entity_Flags flags,
                    vec3 position, vec3 rotation, vec3 scale, char *mesh_file) {

  Entity *entity = pool_alloc(&ep->pool);

  *entity = (Entity){
      .id = ep->next_entity_id,
      .flags = flags,
      .position = position,
      .rotation = rotation,
      .scale = scale,
      .mesh_asset = ass_load_mesh_obj(am, rc, mesh_file),
  };

  // and increment the id
  ep->next_entity_id++;

  return entity;
}

mat4 entity_model_transform(const Entity *entity) {
  // mat4 transform = mat4_mul(mat4_translation(entity->position),
  //                           mat4_mul(mat4_rotation_y(entity->rotation.y),
  //                                    mat4_mul(mat4_rotation_x(entity->rotation.x),
  //                                             mat4_mul(mat4_rotation_z(entity->rotation.z),
  //                                                      mat4_scale(entity->scale)))));

  // Taken algebra from Brendan Galea, couldn't be bothered, tait bryan angles, Y, X, Z
  f32 sinx = sinf(entity->rotation.x);
  f32 cosx = cosf(entity->rotation.x);
  f32 siny = sinf(entity->rotation.y);
  f32 cosy = cosf(entity->rotation.y);
  f32 sinz = sinf(entity->rotation.z);
  f32 cosz = cosf(entity->rotation.z);

  // We can algebraically simplify the above like so, throwing it into godbolt,
  // it turned ~500 ASM instructions into ~100, simulating just his transform for 10,000
  // entities this is faster on my computer by about 20 fps
  mat4 transform = {.cols = {
                        {
                            .x = entity->scale.x * (cosy * cosz + siny * sinx * sinz),
                            .y = entity->scale.x * (cosx * sinz),
                            .z = entity->scale.x * (cosy * sinx * sinz - cosz * siny),
                            .w = 0.0f,
                        },
                        {
                            .x = entity->scale.y * (cosz * siny * sinx - cosy * sinz),
                            .y = entity->scale.y * (cosx * cosz),
                            .z = entity->scale.y * (cosy * cosz * sinx + siny * sinz),
                            .w = 0.0f,
                        },
                        {
                            .x = entity->scale.z * (cosx * siny),
                            .y = entity->scale.z * (-sinx),
                            .z = entity->scale.z * (cosy * cosx),
                            .w = 0.0f,
                        },
                        {
                            .xyz = entity->position,
                            .w = 1.0f,
                        },
                    }};
  return transform;
}

mat4 entity_normal_matrix(const Entity *entity) {
  // Taken algebra from Brendan Galea, couldn't be bothered, tait bryan angles, Y, X, Z
  f32 sinx = sinf(entity->rotation.x);
  f32 cosx = cosf(entity->rotation.x);
  f32 siny = sinf(entity->rotation.y);
  f32 cosy = cosf(entity->rotation.y);
  f32 sinz = sinf(entity->rotation.z);
  f32 cosz = cosf(entity->rotation.z);

  vec3 inv_scale = vec3_inv(entity->scale);

  // Optimization... we don't need the translation, and the inverse transopse of a rotation is that
  // same rotation since inverse of rotation mat is it's transopse, the transpose of a scale matrix
  // is itself, and the inverse is easy, like the above
  mat4 normal = {.cols = {
                     {
                         .x = inv_scale.x * (cosy * cosz + siny * sinx * sinz),
                         .y = inv_scale.x * (cosx * sinz),
                         .z = inv_scale.x * (cosy * sinx * sinz - cosz * siny),
                         .w = 0.0f,
                     },
                     {
                         .x = inv_scale.y * (cosz * siny * sinx - cosy * sinz),
                         .y = inv_scale.y * (cosx * cosz),
                         .z = inv_scale.y * (cosy * cosz * sinx + siny * sinz),
                         .w = 0.0f,
                     },
                     {
                         .x = inv_scale.z * (cosx * siny),
                         .y = inv_scale.z * (-sinx),
                         .z = inv_scale.z * (cosy * cosx),
                         .w = 0.0f,
                     },
                     {
                         .elements = {0.0f},
                     },
                 }};
  return normal;
}

void entity_free(Entity_Pool *entity_pool, Entity *entity) {
  pool_pop(&entity_pool->pool, entity);
  entity->flags = ENTITY_FLAG_INVALID; // This feels icky to do this
}
