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
from gapit_test_framework import GapitTest
import gapit_test_framework
from struct_offsets import VulkanStruct, UINT32_T, SIZE_T, POINTER
from struct_offsets import HANDLE, FLOAT, CHAR, ARRAY
from vulkan_constants import *


@gapit_test("vkCreateSwapchainKHR_test.apk")
class SwapchainCreateTest(GapitTest):

    def expect(self):
        architecture = require(self.next_call_of("architecture"))
        device_properties = require(
            self.next_call_of("vkGetPhysicalDeviceProperties"))

        createSwapchain = require(self.next_call_of("vkCreateSwapchainKHR"))
        destroySwapchain = require(self.next_call_of("vkDestroySwapchainKHR"))

        swapchain_create_info = VulkanStruct(architecture,
                                             [UINT32_T,  # sType
                                              POINTER,  # pNext
                                              UINT32_T,  # flags
                                              HANDLE,  # surface
                                              UINT32_T,  # minImageCount
                                              UINT32_T,  # imageFormat
                                              UINT32_T,  # imageColorSpace
                                              UINT32_T,  # extent.width
                                              UINT32_T,  # extent.height
                                              UINT32_T,  # imageArrayLayers
                                              UINT32_T,  # imageUsage
                                              UINT32_T,  # imageSharingMode
                                              UINT32_T,  # queueFamilyIndexCount
                                              POINTER,  # queueFamilyIndices
                                              UINT32_T,  # preTransform
                                              UINT32_T,  # compositeAlpha
                                              UINT32_T,  # presentMode
                                              UINT32_T,  # clipped
                                              HANDLE,  # oldSwapchain
                                             ])
        sType = little_endian_bytes_to_int(
            require(
                createSwapchain.get_read_data(
                    createSwapchain.hex_PCreateInfo +
                    swapchain_create_info.get_offset_of(0), 4)))

        require_equal(sType, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR)

        oldSwapchain = little_endian_bytes_to_int(
            require(
                createSwapchain.get_read_data(
                    createSwapchain.hex_PCreateInfo +
                    swapchain_create_info.get_offset_of(18), 8)))
        require_equal(oldSwapchain, 0)

        clipped = little_endian_bytes_to_int(
            require(
                createSwapchain.get_read_data(
                    createSwapchain.hex_PCreateInfo +
                    swapchain_create_info.get_offset_of(17), 4)))
        require_equal(clipped, 0)

        image_array_layers = little_endian_bytes_to_int(
            require(
                createSwapchain.get_read_data(
                    createSwapchain.hex_PCreateInfo +
                    swapchain_create_info.get_offset_of(9), 4)))
        require_equal(image_array_layers, 1)

        require_not_equal(0, destroySwapchain.int_Swapchain)

        queue_family_index_count = little_endian_bytes_to_int(
            require(
                createSwapchain.get_read_data(
                    createSwapchain.hex_PCreateInfo +
                    swapchain_create_info.get_offset_of(12), 4)))
        require_equal(True, (queue_family_index_count == 0 or
                             queue_family_index_count == 2))

        if self.not_device(device_properties, 0x5A400000,
                           gapit_test_framework.PIXEL_C):
            # Our second vkDestroySwapchain should have been called with
            # VK_NULL_HANDLE
            destroySwapchain = require(
                self.next_call_of("vkDestroySwapchainKHR"))
            require_equal(0, destroySwapchain.int_Swapchain)
