# Copyright 2016 Google Inc.
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
#

project(VulkanTestApplications)

cmake_minimum_required(VERSION 3.1)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

include(cmake/setup.cmake)

add_subdirectory(support)
add_subdirectory(vulkan_wrapper)
add_subdirectory(vulkan_helpers)

set(mathfu_build_benchmarks OFF CACHE BOOL "")
set(mathfu_build_tests OFF CACHE BOOL "")
add_subdirectory(third_party/mathfu)

# Support shaders
add_subdirectory(shader_library)

# Support Models
add_subdirectory(standard_models)

# Support Images
add_subdirectory(standard_images)

# All of the support code has to go above here, below here should come all of
# the actual tests.
add_subdirectory(command_buffer_tests)
add_subdirectory(extension_tests)
add_subdirectory(initialization_tests)
add_subdirectory(resource_acquisition_tests)
add_subdirectory(resource_binding_tests)
add_subdirectory(resource_creation_tests)
add_subdirectory(synchronization_tests)
add_subdirectory(traits_query_tests)

# Sample Applications and Sandbox
add_subdirectory(application_sandbox)
