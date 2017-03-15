#version 450

layout(location = 0) out vec4 out_color;
layout (location = 2) in vec4 normal;

void main() {
    out_color = vec4(normalize(normal.xyz), 1.0);
}