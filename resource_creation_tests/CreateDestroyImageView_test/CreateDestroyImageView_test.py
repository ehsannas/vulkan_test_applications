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

from gapit_test_framework import gapit_test, require, require_equal
from gapit_test_framework import require_not_equal, little_endian_bytes_to_int
from gapit_test_framework import GapitTest
from gapit_test_framework import PIXEL_C
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, POINTER, HANDLE

IMAGE_VIEW_CREATE_INFO_ELEMENTS = [
    ("sType", UINT32_T),
    ("pNext", POINTER),
    ("flags", UINT32_T),
    ("image", HANDLE),
    ("viewType", UINT32_T),
    ("format", UINT32_T),
    # These nested struct offsets are correct because all of the alignments
    # are trivially satisfied.
    ("components_r", UINT32_T),
    ("components_g", UINT32_T),
    ("components_b", UINT32_T),
    ("components_a", UINT32_T),
    ("subresourceRange_aspectMask", UINT32_T),
    ("subresourceRange_baseMipLevel", UINT32_T),
    ("subresourceRange_levelCount", UINT32_T),
    ("subresourceRange_baseArrayLayer", UINT32_T),
    ("subresourceRange_layerCount", UINT32_T),
]


def check_create_image_view(test, index):
    """Gets the |index|'th vkCreateImageView command call atom, checks its
    return value and arguments. Also checks the image view handler value pointed
    by the image view pointer. This method does not check the content of the
    VkImageViewCreateInfo struct pointed by the create info pointer. Returns the
    checked vkCreateImageView atom, device handler value and image view handler
    value.  """
    create_image_view = require(test.nth_call_of("vkCreateImageView", index))
    require_equal(VK_SUCCESS, int(create_image_view.return_val))
    device = create_image_view.int_Device
    require_not_equal(0, device)
    p_create_info = create_image_view.hex_PCreateInfo
    require_not_equal(0, p_create_info)
    p_image_view = create_image_view.hex_PView
    require_not_equal(0, p_image_view)
    image_view = little_endian_bytes_to_int(require(
        create_image_view.get_write_data(p_image_view,
                                         NON_DISPATCHABLE_HANDLE_SIZE)))
    require_not_equal(0, image_view)
    return create_image_view, device, image_view


def check_destroy_image_view(test, device, image_view, device_properties):
    """Checks the |index|'th vkDestroyImageView command call atom, including the
    device handler value and the image view handler value.
    """
    destroy_image_view = require(test.next_call_of("vkDestroyImageView"))
    require_equal(device, destroy_image_view.int_Device)
    require_equal(image_view, destroy_image_view.int_ImageView)
    if test.not_device(device_properties, 0x5A400000, PIXEL_C):
        # Our second vkDestroyImageView should have been called with
        # VK_NULL_HANDLE
        destroy_image_view_2 = require(test.next_call_of("vkDestroyImageView"))
        require_equal(0, destroy_image_view_2.int_ImageView)


def get_image_view_create_info(create_image_view, architecture):
    """Returns a VulkanStruct which is built to represent the image view create
    info struct used in the given create image view command."""

    def get_data(offset, size):
        return little_endian_bytes_to_int(require(
            create_image_view.get_read_data(create_image_view.hex_PCreateInfo +
                                            offset, size)))

    return VulkanStruct(architecture, IMAGE_VIEW_CREATE_INFO_ELEMENTS,
                        get_data)


@gapit_test("CreateDestroyImageView_test.apk")
class ColorAttachmentImage(GapitTest):
    def expect(self):
        """1. Expects a normal image view for a normal 2D color attachement
        image created in swapchain."""

        architecture = self.architecture
        device_properties = require(self.next_call_of(
            "vkGetPhysicalDeviceProperties"))
        create_image_view, device, image = check_create_image_view(self, 1)
        info = get_image_view_create_info(create_image_view, architecture)
        check_destroy_image_view(self, device, image, device_properties)

        require_equal(info.sType, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO)
        require_equal(info.pNext, 0)
        require_equal(info.flags, 0)
        require_equal(info.viewType, VK_IMAGE_VIEW_TYPE_2D)
        require_equal(info.format, VK_FORMAT_R8G8B8A8_UNORM)
        require_equal(info.components_r, VK_COMPONENT_SWIZZLE_IDENTITY)
        require_equal(info.components_g, VK_COMPONENT_SWIZZLE_IDENTITY)
        require_equal(info.components_b, VK_COMPONENT_SWIZZLE_IDENTITY)
        require_equal(info.components_a, VK_COMPONENT_SWIZZLE_IDENTITY)
        require_equal(info.subresourceRange_aspectMask, VK_IMAGE_ASPECT_COLOR_BIT)
        require_equal(info.subresourceRange_baseMipLevel, 0)
        require_equal(info.subresourceRange_levelCount, 1)
        require_equal(info.subresourceRange_baseArrayLayer, 0)
        require_equal(info.subresourceRange_layerCount, 1)
