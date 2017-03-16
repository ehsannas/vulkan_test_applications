#version 450

layout(location = 0) out vec4 out_color;
layout (location = 2) in vec2 tex_coord;

void main() {
    out_color = vec4(tex_coord, 0.0, 1.0);
}