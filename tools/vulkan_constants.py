#!/usr/bin/python
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
"""This contains a bunch of named vulkan constants."""

VK_NULL_HANDLE = 0

VK_SUCCESS = 0
VK_INCOMPLETE = 5

NON_DISPATCHABLE_HANDLE_SIZE = 8

# VkStructureType
VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1
VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO = 9
VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO = 14
VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO = 15
VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO = 38
VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO = 41
VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO = 42
VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER = 45
VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR = 1000008000
VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR = 1000001000

# VkImageType
VK_IMAGE_TYPE_1D = 0
VK_IMAGE_TYPE_2D = 1
VK_IMAGE_TYPE_3D = 2

# VkImageCreateFlagBits
VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT = 0x00000008
VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT = 0x00000010

# VkFormat
VK_FORMAT_R8G8B8A8_UNORM = 37
VK_FORMAT_D16_UNORM = 124

# VkSampleCountFlagBits
VK_SAMPLE_COUNT_1_BIT = 0x00000001
VK_SAMPLE_COUNT_4_BIT = 0x00000004

# VkImageTiling
VK_IMAGE_TILING_OPTIMAL = 0
VK_IMAGE_TILING_LINEAR = 1

# VkImageUsageFlagBits
VK_IMAGE_USAGE_TRANSFER_SRC_BIT = 0x00000001
VK_IMAGE_USAGE_TRANSFER_DST_BIT = 0x00000002
VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT = 0x00000010
VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT = 0x00000020

# VkSharingMode
VK_SHARING_MODE_EXCLUSIVE = 0
VK_SHARING_MODE_CONCURRENT = 1

# VkImageLayout
VK_IMAGE_LAYOUT_UNDEFINED = 0
VK_IMAGE_LAYOUT_PREINITIALIZED = 8

# VkPipelineStageFlags
VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT = 0x00000001
VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT = 0x00000002
VK_PIPELINE_STAGE_VERTEX_INPUT_BIT = 0x00000004
VK_PIPELINE_STAGE_VERTEX_SHADER_BIT = 0x00000008
VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT = 0x00000010
VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT = 0x00000020
VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT = 0x00000040
VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT = 0x00000080
VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT = 0x00000100
VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT = 0x00000200
VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT = 0x00000400
VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT = 0x00000800
VK_PIPELINE_STAGE_TRANSFER_BIT = 0x00001000
VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT = 0x00002000
VK_PIPELINE_STAGE_HOST_BIT = 0x00004000
VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT = 0x00008000
VK_PIPELINE_STAGE_ALL_COMMANDS_BIT = 0x00010000

# VkDependencyFlags
VK_DEPENDENCY_BY_REGION_BIT = 0x00000001

# VkAccessFlags
VK_ACCESS_INDIRECT_COMMAND_READ_BIT = 0x00000001
VK_ACCESS_INDEX_READ_BIT = 0x00000002
VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT = 0x00000004
VK_ACCESS_UNIFORM_READ_BIT = 0x00000008
VK_ACCESS_INPUT_ATTACHMENT_READ_BIT = 0x00000010
VK_ACCESS_SHADER_READ_BIT = 0x00000020
VK_ACCESS_SHADER_WRITE_BIT = 0x00000040
VK_ACCESS_COLOR_ATTACHMENT_READ_BIT = 0x00000080
VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT = 0x00000100
VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT = 0x00000200
VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT = 0x00000400
VK_ACCESS_TRANSFER_READ_BIT = 0x00000800
VK_ACCESS_TRANSFER_WRITE_BIT = 0x00001000
VK_ACCESS_HOST_READ_BIT = 0x00002000
VK_ACCESS_HOST_WRITE_BIT = 0x00004000
VK_ACCESS_MEMORY_READ_BIT = 0x00008000
VK_ACCESS_MEMORY_WRITE_BIT = 0x00010000

# VkImageLayout
VK_IMAGE_LAYOUT_UNDEFINED = 0
VK_IMAGE_LAYOUT_GENERAL = 1
VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL = 2
VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL = 3
VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL = 4
VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL = 5
VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL = 6
VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL = 7
VK_IMAGE_LAYOUT_PREINITIALIZED = 8
VK_IMAGE_LAYOUT_PRESENT_SRC_KHR = 1000001002

# VkImageAspectFlags
VK_IMAGE_ASPECT_COLOR_BIT = 0x00000001
VK_IMAGE_ASPECT_DEPTH_BIT = 0x00000002
VK_IMAGE_ASPECT_STENCIL_BIT = 0x00000004
VK_IMAGE_ASPECT_METADATA_BIT = 0x00000008

# VkImageViewType
VK_IMAGE_VIEW_TYPE_1D = 0
VK_IMAGE_VIEW_TYPE_2D = 1
VK_IMAGE_VIEW_TYPE_2D = 2

# VkComponentSwizzle
VK_COMPONENT_SWIZZLE_IDENTITY = 0

# VkImageAspectFlagBits
VK_IMAGE_ASPECT_COLOR_BIT = 0x00000001

# VkPipelineBindPoint
VK_PIPELINE_BIND_POINT_GRAPHICS = 0
VK_PIPELINE_BIND_POINT_COMPUTE = 1

# VkAttachmentLoadOp
VK_ATTACHMENT_LOAD_OP_LOAD = 0
VK_ATTACHMENT_LOAD_OP_CLEAR = 1
VK_ATTACHMENT_LOAD_OP_DONT_CARE = 2

# VkAttachmentStoreOp
VK_ATTACHMENT_STORE_OP_STORE = 0
VK_ATTACHMENT_STORE_OP_DONT_CARE = 1
