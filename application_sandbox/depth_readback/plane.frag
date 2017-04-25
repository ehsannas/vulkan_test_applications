#version 450

layout(location = 0) out vec4 out_color;

layout(input_attachment_index = 0, binding = 0, set = 0) uniform subpassInputMS depth;

void main() {
    vec4 f = subpassLoad(depth, 1).rrrr;
    out_color = vec4(pow(f.rgb, vec3(10.0, 10.0, 10.0)) , 1.0);
}