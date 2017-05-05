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
from gapit_test_framework import require_not_equal, GapitTest, PIXEL_C
from gapit_test_framework import get_read_offset_function, get_write_offset_function
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, POINTER, HANDLE, DEVICE_SIZE
from struct_offsets import ARRAY, CHAR

BUFFER_VIEW_CREATE_INFO = [
    ("sType", UINT32_T),
    ("pNext", POINTER),
    ("flags", UINT32_T),
    ("buffer", HANDLE),
    ("format", UINT32_T),
    ("offset", DEVICE_SIZE),
    ("range", DEVICE_SIZE),
]

BUFFER_VIEW = [("handle", HANDLE)]

MIN_TEXEL_BUFFER_OFFSET_ALIGNMENT = 16


@gapit_test("CreateDestroyBufferView_test.apk")
class ZeroOffsetWholeSizeBufferViewOfUniformBuffer(GapitTest):

    def expect(self):
        """1. Expects a buffer view created with zero offset and VK_WHOLE_SIZE
        range for a uniform texel buffer."""

        architecture = self.architecture
        device_properties = require(self.next_call_of(
            "vkGetPhysicalDeviceProperties"))

        create_buffer_view = require(self.nth_call_of("vkCreateBufferView", 1))
        device = create_buffer_view.int_Device
        require_not_equal(0, device)
        require_not_equal(0, create_buffer_view.hex_PCreateInfo)
        require_equal(0, create_buffer_view.hex_PAllocator)
        require_not_equal(0, create_buffer_view.hex_PView)
        require_equal(VK_SUCCESS, int(create_buffer_view.return_val))

        create_info = VulkanStruct(
            architecture, BUFFER_VIEW_CREATE_INFO, get_read_offset_function(
                create_buffer_view, create_buffer_view.hex_PCreateInfo))
        require_equal(VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
                      create_info.sType)
        require_equal(0, create_info.pNext)
        require_equal(0, create_info.flags)
        require_not_equal(0, create_info.buffer)
        require_equal(VK_FORMAT_R8G8B8A8_UNORM, create_info.format)
        require_equal(0, create_info.offset)
        require_equal(VK_WHOLE_SIZE, create_info.range)

        view = VulkanStruct(
            architecture, BUFFER_VIEW, get_write_offset_function(
                create_buffer_view, create_buffer_view.hex_PView))
        require_not_equal(0, view.handle)

        destroy_buffer_view = require(self.next_call_of("vkDestroyBufferView"))
        require_equal(device, destroy_buffer_view.int_Device)
        require_equal(view.handle, destroy_buffer_view.int_BufferView)

        if self.not_device(device_properties, 0x5A400000, PIXEL_C):
            destroy_null_buffer_view = require(self.next_call_of(
                "vkDestoryBufferView"))
            require_equal(device, destroy_buffer_view.int_Device)
            require_equal(0, destroy_null_buffer_view.int_BufferView)


@gapit_test("CreateDestroyBufferView_test.apk")
class NonZeroOffsetNonWholeSizeBufferViewOfStorageBuffer(GapitTest):

    def expect(self):
        """2. Expects a buffer view created with non-zero offset and
        non-VK_WHOLE_SIZE range for a storage texel buffer."""

        architecture = self.architecture
        device_properties = require(self.next_call_of(
            "vkGetPhysicalDeviceProperties"))

        create_buffer_view = require(self.nth_call_of("vkCreateBufferView", 2))
        device = create_buffer_view.int_Device
        require_not_equal(0, device)
        require_not_equal(0, create_buffer_view.hex_PCreateInfo)
        require_equal(0, create_buffer_view.hex_PAllocator)
        require_not_equal(0, create_buffer_view.hex_PView)
        require_equal(VK_SUCCESS, int(create_buffer_view.return_val))

        create_info = VulkanStruct(
            architecture, BUFFER_VIEW_CREATE_INFO, get_read_offset_function(
                create_buffer_view, create_buffer_view.hex_PCreateInfo))
        require_equal(VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
                      create_info.sType)
        require_equal(0, create_info.pNext)
        require_equal(0, create_info.flags)
        require_not_equal(0, create_info.buffer)
        require_equal(VK_FORMAT_R8G8B8A8_UNORM, create_info.format)
        require_equal(MIN_TEXEL_BUFFER_OFFSET_ALIGNMENT * 1, create_info.offset)
        require_equal(MIN_TEXEL_BUFFER_OFFSET_ALIGNMENT * 3, create_info.range)

        view = VulkanStruct(
            architecture, BUFFER_VIEW, get_write_offset_function(
                create_buffer_view, create_buffer_view.hex_PView))
        require_not_equal(0, view.handle)

        destroy_buffer_view = require(self.next_call_of("vkDestroyBufferView"))
        require_equal(device, destroy_buffer_view.int_Device)
        require_equal(view.handle, destroy_buffer_view.int_BufferView)

        if self.not_device(device_properties, 0x5A400000, PIXEL_C):
            destroy_null_buffer_view = require(self.next_call_of(
                "vkDestoryBufferView"))
            require_equal(device, destroy_buffer_view.int_Device)
            require_equal(0, destroy_null_buffer_view.int_BufferView)
