#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec3 uv;

layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform Push {
    mat4 clip_transform;
    mat4 normal_matrix;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(0.0, 0.0, 1.0));
const float AMBIENT = 0.1;

void main() {
    gl_Position = push.clip_transform * vec4(position, 1.0);

    // We want the normals in world space... not in model space,
    // therefore transform the normal vertex to world space
    // As well we only need the 3x3 matrix, normals are directions
    // not positions, not affected by translations
    vec3 normal_world_space = normalize(mat3(push.normal_matrix) * normal);

    // We don't really care if the light is facing away, clamp negatives to 0
    float light_intensity = AMBIENT + max(dot(normal_world_space, DIRECTION_TO_LIGHT), 0);

    frag_color = light_intensity * color;
}
