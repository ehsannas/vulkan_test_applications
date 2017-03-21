#version 450
#include "models/model_setup.glsl"

layout (location = 2) out vec3 normal;

layout (binding = 0, set = 0) uniform camera_data {
    layout(column_major) mat4x4 projection;
};

layout (binding = 1, set = 0) uniform model_data {
    layout(column_major) mat4x4 transform;
};

void main() {
    gl_Position =  projection * transform * get_position();
    normal = abs(get_normal().xyz);
}