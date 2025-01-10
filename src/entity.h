#ifndef ENTITY_H
#define ENTITY_H

#include "core/common.h"
#include "core/linear_algebra.h"
#include "render/render_mesh.h"

typedef struct Entity Entity;
struct Entity {
    u32 id;
    u32 flags;

    vec3 position;
    vec3 rotation;
    vec3 scale;
    vec3 color;

    // TODO(ss): Handles into an asset manager, instead
    Render_Mesh *mesh;
};

Entity entity_create(void);

#endif // ENTITY_H
