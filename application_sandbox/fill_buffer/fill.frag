#version 450

layout(location = 0) out vec4 out_color;
layout (location = 2) in vec3 normal;

layout (binding = 2, set = 0) uniform filled_data {
    vec4 multiplier;
};

void main() {
    out_color = vec4(normal * multiplier.xyz, 1.0);
}