#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 out_color;
layout(location = 1) in vec2 texcoord;

layout(set = 0, binding = 2) uniform isamplerBuffer dispatchData;

void main() {
    uint dr = texelFetch(dispatchData, 0).r;
    dr = dr > 128 ? 256 - dr : dr;
    float df = dr / 128.0;
    vec3 color = vec3(texcoord, 1.0) * df;
    out_color = vec4(color, 1.0);
}
