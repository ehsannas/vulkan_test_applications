#version 450

layout(location = 0) out vec4 out_color;
layout (location = 1) in vec2 texcoord;

layout(set = 0, binding = 2) uniform sampler default_sampler;
layout(set = 0, binding = 3) uniform texture2D default_texture;

void main() {
    vec4 color = texture(sampler2D(default_texture, default_sampler), texcoord);
    out_color = vec4(color.xyz, 1.0);
}