
layout(location = 0) in vec3 _position;
layout(location = 1) in vec2 _texture_coord;
layout(location = 2) in vec3 _normal;

vec4 get_position() {
    return vec4(_position, 1.0);
}

vec2 get_texcoord() {
    return _texture_coord;
}

vec4 get_normal() {
    return vec4(_normal, 1.0);
}