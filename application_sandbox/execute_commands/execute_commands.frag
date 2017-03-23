#version 450

layout(location = 0) out vec4 out_color;
layout(location = 1) in vec2 texcoord;

layout(set = 0, binding = 2) uniform samplerBuffer alphaData;

void main() {
    float a = texelFetch(alphaData, 0).r;
    float b = a;
    if (b > 1.0) {
      b = 2.0 - b;
    }
    vec3 color = vec3(texcoord, 1.0) * b;
    out_color = vec4(color, 1.0);
}
