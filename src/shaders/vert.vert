#version 450

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;

layout(location = 0) out vec3 frag_color;

layout(push_constant) uniform Push {
    vec3 offset;
    vec3 color;
} push;

void main() {
    gl_Position = vec4(position + push.offset.xy, 0.0, 1.0);
    frag_color = color;
}
