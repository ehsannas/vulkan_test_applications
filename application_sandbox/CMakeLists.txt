# Copyright 2017 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


# Everything else depends on sample_application
add_subdirectory(sample_application_framework)

add_custom_target(ALL_SAMPLES)

function(add_vulkan_sample_application name)
    add_vulkan_executable(${name} NON_DEFAULT ${ARGN})
    add_dependencies(ALL_SAMPLES ${name})
endfunction()

add_subdirectory(blend_constants)
add_subdirectory(blit_image)
add_subdirectory(bufferview)
add_subdirectory(clear_attachments)
add_subdirectory(clear_colorimage)
add_subdirectory(clear_depthimage)
add_subdirectory(cube)
add_subdirectory(depth_readback)
add_subdirectory(textured_cube)
add_subdirectory(wireframe)