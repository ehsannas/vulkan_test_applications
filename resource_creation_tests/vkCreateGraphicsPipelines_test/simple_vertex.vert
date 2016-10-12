/* Copyright 2016 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#version 450

layout(std140, binding = 0) uniform buff {
    mat4 MPV_Matrix;
};

layout (location = 0) in vec4 vertexCoord;
layout (location = 1) in vec2 itexCoord;
layout (location = 0) out vec2 otexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = MPV_Matrix * vertexCoord;
    otexCoord = itexCoord;
}