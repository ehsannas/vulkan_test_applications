#version 450

layout(location = 0) out vec4 out_color;
layout (location = 2) in vec4 normal;

void main() {


    vec3 norm = normalize(normal.xyz);
    vec3 stripes = mix(vec3(0),
        norm * 0.5, lessThan(mod(gl_FragCoord.xyz, vec3(4.0)), vec3(2.0)));

    out_color = vec4(mix(stripes, norm, bvec3(gl_FrontFacing)), 1.0);
}