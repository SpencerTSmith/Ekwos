#include "game/entity.h"
#include "core/arena.h"

Entity_Pool entity_pool_init(u64 capacity) {
    Entity_Pool pool = {
        .arena = arena_create(sizeof(Entity) * capacity, ARENA_FLAG_DEFAULTS),
        .next_entity_id = 1, // again id of 0 is invalid
    };

    return pool;
}
Entity *entity_create(Entity_Pool *entity_pool, Entity_Flags flags, vec3 position, vec3 rotation,
                      vec3 scale, vec3 color, RND_Mesh *mesh) {

    Entity *entity = arena_calloc(&entity_pool->arena, 1, Entity);

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
    mat4 transform = mat4_identity();

    return transform;
}

void entity_free(Entity_Pool *entity_pool, Entity *entity) {
    u32 position = (entity->id - 1) * sizeof(Entity);
}
