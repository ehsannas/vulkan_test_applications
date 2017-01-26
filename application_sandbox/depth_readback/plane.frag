#version 450

layout(location = 0) out vec4 out_color;

layout(input_attachment_index = 0, binding = 0, set = 0) uniform subpassInputMS depth;

void main() {
    out_color = pow(subpassLoad(depth, 1), vec4(10.0, 10.0, 10.0, 1.0));
}