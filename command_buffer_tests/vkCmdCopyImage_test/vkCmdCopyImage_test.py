# Copyright 2016 Google Inc.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#     http://www.apache.org/licenses/LICENSE-2.0
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from gapit_test_framework import gapit_test, require, require_equal
from gapit_test_framework import require_not_equal, little_endian_bytes_to_int
from gapit_test_framework import GapitTest, get_read_offset_function
import gapit_test_framework
from struct_offsets import VulkanStruct, UINT32_T, SIZE_T, POINTER
from struct_offsets import HANDLE, FLOAT, CHAR, ARRAY, DEVICE_SIZE, INT32_T
from vulkan_constants import *

IMAGE_COPY = [
    ("srcSubresource_aspectMask", UINT32_T),
    ("srcSubresource_mipLevel", UINT32_T),
    ("srcSubresource_baseArrayLayer", UINT32_T),
    ("srcSubresource_layerCount", UINT32_T),
    ("srcOffset_x", UINT32_T),
    ("srcOffset_y", UINT32_T),
    ("srcOffset_z", UINT32_T),
    ("dstSubresource_aspectMask", UINT32_T),
    ("dstSubresource_mipLevel", UINT32_T),
    ("dstSubresource_baseArrayLayer", UINT32_T),
    ("dstSubresource_layerCount", UINT32_T),
    ("dstOffset_x", UINT32_T),
    ("dstOffset_y", UINT32_T),
    ("dstOffset_z", UINT32_T),
    ("extent_x", UINT32_T),
    ("extent_y", UINT32_T),
    ("extent_z", UINT32_T),
]


@gapit_test("vkCmdCopyImage_test.apk")
class CopyWholeColorImageToSameColorImageOneLayer(GapitTest):

    def expect(self):
        """Check the arguments to vkCmdCopyImage"""
        architecture = require(self.next_call_of("architecture"))
        copy_image = require(self.nth_call_of("vkCmdCopyImage", 1))

        require_not_equal(0, copy_image.int_CommandBuffer)
        require_not_equal(0, copy_image.int_SrcImage)
        require_equal(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      copy_image.int_SrcImageLayout)
        require_not_equal(0, copy_image.int_DstImage)
        require_equal(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      copy_image.int_DstImageLayout)
        require_equal(1, copy_image.int_RegionCount)

        image_copy = VulkanStruct(architecture, IMAGE_COPY,
                                  get_read_offset_function(
                                      copy_image, copy_image.hex_PRegions))

        require_equal(VK_IMAGE_ASPECT_COLOR_BIT,
                      image_copy.srcSubresource_aspectMask)
        require_equal(0, image_copy.srcSubresource_mipLevel)
        require_equal(0, image_copy.srcSubresource_baseArrayLayer)
        require_equal(1, image_copy.srcSubresource_layerCount)
        require_equal(0, image_copy.srcOffset_x)
        require_equal(0, image_copy.srcOffset_y)
        require_equal(0, image_copy.srcOffset_z)
        require_equal(VK_IMAGE_ASPECT_COLOR_BIT,
                      image_copy.dstSubresource_aspectMask)
        require_equal(0, image_copy.dstSubresource_mipLevel)
        require_equal(0, image_copy.dstSubresource_baseArrayLayer)
        require_equal(1, image_copy.dstSubresource_layerCount)
        require_equal(0, image_copy.dstOffset_x)
        require_equal(0, image_copy.dstOffset_y)
        require_equal(0, image_copy.dstOffset_z)
        require_equal(32, image_copy.extent_x)
        require_equal(32, image_copy.extent_y)
        require_equal(1, image_copy.extent_z)



@gapit_test("vkCmdCopyImage_test.apk")
class CopyCompressedImageRegionToCompatibleImageRegion(GapitTest):

    def expect(self):
        """Check the arguments to vkCmdCopyImage"""
        architecture = require(self.next_call_of("architecture"))
        copy_image = require(self.nth_call_of("vkCmdCopyImage", 2))

        require_not_equal(0, copy_image.int_CommandBuffer)
        require_not_equal(0, copy_image.int_SrcImage)
        require_equal(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                      copy_image.int_SrcImageLayout)
        require_not_equal(0, copy_image.int_DstImage)
        require_equal(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                      copy_image.int_DstImageLayout)
        require_equal(1, copy_image.int_RegionCount)

        image_copy = VulkanStruct(architecture, IMAGE_COPY,
                                  get_read_offset_function(
                                      copy_image, copy_image.hex_PRegions))

        require_equal(VK_IMAGE_ASPECT_COLOR_BIT,
                      image_copy.srcSubresource_aspectMask)
        require_equal(0, image_copy.srcSubresource_mipLevel)
        require_equal(0, image_copy.srcSubresource_baseArrayLayer)
        require_equal(1, image_copy.srcSubresource_layerCount)
        require_equal(8, image_copy.srcOffset_x)
        require_equal(12, image_copy.srcOffset_y)
        require_equal(0, image_copy.srcOffset_z)
        require_equal(VK_IMAGE_ASPECT_COLOR_BIT,
                      image_copy.dstSubresource_aspectMask)
        require_equal(0, image_copy.dstSubresource_mipLevel)
        require_equal(0, image_copy.dstSubresource_baseArrayLayer)
        require_equal(1, image_copy.dstSubresource_layerCount)
        require_equal(16, image_copy.dstOffset_x)
        require_equal(16, image_copy.dstOffset_y)
        require_equal(0, image_copy.dstOffset_z)
        require_equal(16, image_copy.extent_x)
        require_equal(12, image_copy.extent_y)
        require_equal(1, image_copy.extent_z)
