#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 out_color;
layout(location = 2) in vec4 normal;

layout(set = 0, binding = 2) uniform isamplerBuffer query_data;

void main() {
    int q = texelFetch(query_data, 0).r % 40000;
    float qr = q;
    if (q != 0.0) {
      int nq = 40000 - q;
      qr = (nq < q ? nq : q) / 20000.0;
      qr = qr < 0.3 ? 0.3 : qr;
    }
    out_color = vec4(vec3(qr), 1.0);
}
