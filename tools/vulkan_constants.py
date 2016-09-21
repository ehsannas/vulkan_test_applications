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

NON_DISPATCHABLE_HANDLE_SIZE = 8

# VkStructureType
VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO = 1
VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO = 9
VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO = 14
VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO = 41
VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO = 42
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
