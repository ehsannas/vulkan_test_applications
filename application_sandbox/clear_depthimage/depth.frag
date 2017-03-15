#version 450

layout(location = 0) out vec4 out_color;

layout(input_attachment_index = 0, binding = 0, set = 0) uniform subpassInput depth;

void main() {
    out_color = (subpassLoad(depth)-0.9)*30.0;
}

