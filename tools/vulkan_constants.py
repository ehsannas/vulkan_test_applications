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
VK_STRUCTURE_TYPE_SUBMIT_INFO = 4
VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO = 5
VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO = 9
VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO = 12
VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO = 14
VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO = 15
VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO = 16
VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO = 17
VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO = 18
VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO = 19
VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO = 20
VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO = 21
VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO = 22
VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO = 23
VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO = 24
VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO = 25
VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO = 26
VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO = 27
VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO = 28
VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO = 30
VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO = 31
VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO = 32
VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO = 33
VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO = 37
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
VK_FORMAT_R32G32_SFLOAT = 103
VK_FORMAT_R32G32B32A32_SFLOAT = 109
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
VK_IMAGE_VIEW_TYPE_3D = 2

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

# VkFilter
VK_FILTER_NEAREST = 0
VK_FILTER_LINEAR = 1

# VkSamplerMipmapMode
VK_SAMPLER_MIPMAP_MODE_NEAREST = 0
VK_SAMPLER_MIPMAP_MODE_LINEAR = 1

# VkSamplerAddressMode
VK_SAMPLER_ADDRESS_MODE_REPEAT = 0
VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2
VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER = 3

# VkCompareOp
VK_COMPARE_OP_LESS = 1
VK_COMPARE_OP_ALWAYS = 7

# VkBorderColor
VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK = 2
VK_BORDER_COLOR_INT_OPAQUE_WHITE = 5

# VkBufferUsageBits
VK_BUFFER_USAGE_TRANSFER_SRC_BIT = 0x00000001
VK_BUFFER_USAGE_TRANSFER_DST_BIT = 0x00000002
VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT = 0x00000004
VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT = 0x00000008
VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT = 0x00000010
VK_BUFFER_USAGE_STORAGE_BUFFER_BIT = 0x00000020
VK_BUFFER_USAGE_INDEX_BUFFER_BIT = 0x00000040
VK_BUFFER_USAGE_VERTEX_BUFFER_BIT = 0x00000080
VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT = 0x00000100

VK_WHOLE_SIZE = 0xFFFFFFFFFFFFFFFF

# VkDescriptorType
VK_DESCRIPTOR_TYPE_SAMPLER = 0
VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE = 2
VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3
VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6
VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT = 10

# VkDescriptorPoolCreateFlagBits
VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT = 0x00000001

# VkDescriptorType
VK_DESCRIPTOR_TYPE_STORAGE_IMAGE = 3
VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER = 6
VK_DESCRIPTOR_TYPE_STORAGE_BUFFER = 7

# VkShaderStageFlagBits
VK_SHADER_STAGE_VERTEX_BIT = 0x00000001
VK_SHADER_STAGE_FRAGMENT_BIT = 0x00000010
VK_SHADER_STAGE_ALL = 0x7FFFFFFF

# VkVertexInputRate
VK_VERTEX_INPUT_RATE_VERTEX = 0
VK_VERTEX_INPUT_RATE_INSTANCE = 1

# VkPrimitiveTopology
VK_PRIMITIVE_TOPOLOGY_POINT_LIST = 0
VK_PRIMITIVE_TOPOLOGY_LINE_LIST = 1
VK_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2
VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3
VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4
VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 5
VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY = 6
VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY = 7
VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY = 8
VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY = 9
VK_PRIMITIVE_TOPOLOGY_PATCH_LIST = 10

# VkPolygonMode
VK_POLYGON_MODE_FILL = 0
VK_POLYGON_MODE_LINE = 1
VK_POLYGON_MODE_POINT = 2

# VkCullMode
VK_CULL_MODE_NONE = 0
VK_CULL_MODE_FRONT_BIT = 0x00000001
VK_CULL_MODE_BACK_BIT = 0x00000002
VK_CULL_MODE_FRONT_AND_BACK = 0x00000003

# VkFrontFace
VK_FRONT_FACE_COUNTER_CLOCKWISE = 0
VK_FRONT_FACE_CLOCKWISE = 1

# VkStencilOp
VK_STENCIL_OP_KEEP = 0
VK_STENCIL_OP_ZERO = 1
VK_STENCIL_OP_REPLACE = 2
VK_STENCIL_OP_INCREMENT_AND_CLAMP = 3
VK_STENCIL_OP_DECREMENT_AND_CLAMP = 4
VK_STENCIL_OP_INVERT = 5
VK_STENCIL_OP_INCREMENT_AND_WRAP = 6
VK_STENCIL_OP_DECREMENT_AND_WRAP = 7

# VkCompareOp
VK_COMPARE_OP_NEVER = 0
VK_COMPARE_OP_LESS = 1
VK_COMPARE_OP_EQUAL = 2
VK_COMPARE_OP_LESS_OR_EQUAL = 3
VK_COMPARE_OP_GREATER = 4
VK_COMPARE_OP_NOT_EQUAL = 5
VK_COMPARE_OP_GREATER_OR_EQUAL = 6
VK_COMPARE_OP_ALWAYS = 7

# VkLogicOp
VK_LOGIC_OP_CLEAR = 0
VK_LOGIC_OP_AND = 1
VK_LOGIC_OP_AND_REVERSE = 2
VK_LOGIC_OP_COPY = 3
VK_LOGIC_OP_AND_INVERTED = 4
VK_LOGIC_OP_NO_OP = 5
VK_LOGIC_OP_XOR = 6
VK_LOGIC_OP_OR = 7
VK_LOGIC_OP_NOR = 8
VK_LOGIC_OP_EQUIVALENT = 9
VK_LOGIC_OP_INVERT = 10
VK_LOGIC_OP_OR_REVERSE = 11
VK_LOGIC_OP_COPY_INVERTED = 12
VK_LOGIC_OP_OR_INVERTED = 13
VK_LOGIC_OP_NAND = 14
VK_LOGIC_OP_SET = 15

# VkBlendFactor
VK_BLEND_FACTOR_ZERO = 0
VK_BLEND_FACTOR_ONE = 1
VK_BLEND_FACTOR_SRC_COLOR = 2
VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR = 3
VK_BLEND_FACTOR_DST_COLOR = 4
VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR = 5
VK_BLEND_FACTOR_SRC_ALPHA = 6
VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA = 7
VK_BLEND_FACTOR_DST_ALPHA = 8
VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA = 9
VK_BLEND_FACTOR_CONSTANT_COLOR = 10
VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR = 11
VK_BLEND_FACTOR_CONSTANT_ALPHA = 12
VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA = 13
VK_BLEND_FACTOR_SRC_ALPHA_SATURATE = 14
VK_BLEND_FACTOR_SRC1_COLOR = 15
VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR = 16
VK_BLEND_FACTOR_SRC1_ALPHA = 17
VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA = 18

# VkBlendOp
VK_BLEND_OP_ADD = 0
VK_BLEND_OP_SUBTRACT = 1
VK_BLEND_OP_REVERSE_SUBTRACT = 2
VK_BLEND_OP_MIN = 3
VK_BLEND_OP_MAX = 4

# VkColorComponentBits
VK_COLOR_COMPONENT_R_BIT = 0x00000001
VK_COLOR_COMPONENT_G_BIT = 0x00000002
VK_COLOR_COMPONENT_B_BIT = 0x00000004
VK_COLOR_COMPONENT_A_BIT = 0x00000008