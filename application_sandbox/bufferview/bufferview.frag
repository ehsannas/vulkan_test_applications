#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 out_color;
layout(location = 1) in vec2 texcoord;

layout(set = 0, binding = 2) uniform samplerBuffer alphaData;

void main() {
    float a = texelFetch(alphaData, 0).r;
    float b = a;
    if (b > 1.0) {
      b = 2.0 - b;
    }
    out_color = vec4(texcoord, b, 1.0);
}
