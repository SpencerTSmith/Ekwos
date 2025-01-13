#include "game/entity.h"
#include "core/arena.h"

Entity_Pool entity_pool_init(u64 capacity) {
    Entity_Pool pool = {
        .pool = pool_create_type(capacity, Entity),
        .next_entity_id = 1, // again id of 0 is invalid
    };

    return pool;
}

Entity *entity_create(Entity_Pool *entity_pool, Entity_Flags flags, vec3 position, vec3 rotation,
                      vec3 scale, vec3 color, RND_Mesh *mesh) {

    Entity *entity = pool_alloc(&entity_pool->pool);

    *entity = (Entity){
        .id = entity_pool->next_entity_id,
        .flags = flags,
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .color = color,
        .mesh = mesh,
    };

    // and increment the id
    entity_pool->next_entity_id += 1;

    return entity;
}

mat4 entity_world_transform(Entity *entity) {
    // mat4 transform = mat4_mul(mat4_translation(entity->position),
    //                           mat4_mul(mat4_make_rotation_y(entity->rotation.y),
    //                                    mat4_mul(mat4_make_rotation_x(entity->rotation.x),
    //                                             mat4_mul(mat4_make_rotation_z(entity->rotation.z),
    //                                                      mat4_scale(entity->scale)))));

    // Taken algebra from Brendan Galea, couldn't be bothered
    f32 sinx = sinf(entity->rotation.x);
    f32 cosx = cosf(entity->rotation.x);
    f32 siny = sinf(entity->rotation.y);
    f32 cosy = cosf(entity->rotation.y);
    f32 sinz = sinf(entity->rotation.z);
    f32 cosz = cosf(entity->rotation.z);

    // We can algebraically simplify the above like so, throwing it into godbolt,
    // it turned ~500 ASM instructions into ~100, simulating just his transform for 10,000 entities
    // this is faster on my computer by about 20 fps
    mat4 transform = {.cols = {
                          {
                              .x = entity->scale.x * (cosx * cosz + sinx * siny * sinz),
                              .y = entity->scale.x * (cosy * sinz),
                              .z = entity->scale.x * (cosx * siny * sinz - cosz * sinx),
                              .w = 0.0f,
                          },
                          {
                              .x = entity->scale.y * (cosz * sinx * siny - cosx * sinz),
                              .y = entity->scale.y * (cosy * cosz),
                              .z = entity->scale.y * (cosx * cosz * siny + sinx * sinz),
                              .w = 0.0f,
                          },
                          {
                              .x = entity->scale.z * (cosy * sinx),
                              .y = entity->scale.z * (-siny),
                              .z = entity->scale.z * (cosx * cosy),
                              .w = 0.0f,
                          },
                          {
                              .xyz = entity->position,
                              .w = 1.0f,
                          },
                      }};
    return transform;
}

void entity_free(Entity_Pool *entity_pool, Entity *entity) { pool_pop(&entity_pool->pool, entity); }
