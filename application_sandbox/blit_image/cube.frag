#version 450

layout(location = 0) out vec4 out_color;
layout (location = 1) in vec2 texcoord;



void main() {
    out_color = vec4(texcoord, 0.0, 1.0);
}