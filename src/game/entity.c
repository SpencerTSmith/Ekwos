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

mat4 entity_transform(Entity *entity) {
    mat4 transform = mat4_mul(mat4_make_translation(entity->position),
                              mat4_mul(mat4_make_rotation_y(entity->rotation.y),
                                       mat4_mul(mat4_make_rotation_x(entity->rotation.x),
                                                mat4_mul(mat4_make_rotation_z(entity->rotation.z),
                                                         mat4_make_scale(entity->scale)))));

    return transform;
}

void entity_free(Entity_Pool *entity_pool, Entity *entity) { pool_pop(&entity_pool->pool, entity); }
