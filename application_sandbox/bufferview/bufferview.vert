#version 450
#include "models/model_setup.glsl"

layout (location = 1) out vec2 texcoord;

layout (binding = 0, set = 0) uniform camera_data {
    layout(column_major) mat4x4 projection;
};

layout (binding = 1, set = 0) uniform model_data {
    layout(column_major) mat4x4 transform;
};

void main() {
    gl_Position =  projection * transform * get_position();
    texcoord = get_texcoord();
}
