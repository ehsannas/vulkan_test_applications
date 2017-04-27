#version 450
#include "models/model_setup.glsl"
#include "particle_data_shared.h"

layout (location = 1) out vec2 texcoord;
layout (location = 2) out float speed;

layout (binding = 0) buffer DrawData {
  draw_data drawData[TOTAL_PARTICLES];
};

layout (binding = 3) buffer frame {
    Vector4 aspect_data;
};

void main() {
    vec4 position = get_position() / 250.0f;
    gl_Position =
        vec4(position.xy + drawData[gl_InstanceIndex].position_speed.xy, 0.0f, 1.0f);
    gl_Position.x /= aspect_data.x;
    texcoord = get_texcoord();
    speed = length(drawData[gl_InstanceIndex].position_speed.zw);
}