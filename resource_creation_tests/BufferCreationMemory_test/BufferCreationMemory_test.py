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
from gapit_test_framework import get_write_offset_function
import gapit_test_framework
from struct_offsets import VulkanStruct, UINT32_T, DEVICE_SIZE, SIZE_T, POINTER
from struct_offsets import HANDLE, FLOAT, CHAR, ARRAY
from vulkan_constants import *

BUFFER_CREATE_INFO = [
    ("sType", UINT32_T), ("pNext", POINTER), ("createFlags", UINT32_T),
    ("size", DEVICE_SIZE), ("usage", UINT32_T), ("sharingMode", UINT32_T),
    ("queueFamilyIndexCount", UINT32_T), ("pQueueFamilyIndices", POINTER)
]

MEMORY_REQUIREMENTS = [
    ("size", DEVICE_SIZE), ("alignment", DEVICE_SIZE),
    ("memoryTypeBits", UINT32_T)
]


@gapit_test("BufferCreationMemory_test.apk")
class BufferCreationDestroy(GapitTest):

    def expect(self):
        architecture = require(self.next_call_of("architecture"))
        device_properties = require(
            self.next_call_of("vkGetPhysicalDeviceProperties"))

        create_buffer = require(self.next_call_of("vkCreateBuffer"))

        buffer_create_info = VulkanStruct(architecture, BUFFER_CREATE_INFO,
                                          get_read_offset_function(
                                              create_buffer,
                                              create_buffer.hex_PCreateInfo))

        written_buffer = little_endian_bytes_to_int(
            require(
                create_buffer.get_write_data(create_buffer.hex_PBuffer,
                                             NON_DISPATCHABLE_HANDLE_SIZE)))

        require_not_equal(0, written_buffer)

        require_equal(buffer_create_info.sType,
                      VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO)
        require_equal(buffer_create_info.pNext, 0)
        require_equal(buffer_create_info.createFlags, 0)
        require_equal(buffer_create_info.size, 1024)
        require_equal(buffer_create_info.usage,
                      VK_BUFFER_USAGE_TRANSFER_SRC_BIT)
        require_equal(buffer_create_info.sharingMode, VK_SHARING_MODE_EXCLUSIVE)
        require_equal(buffer_create_info.queueFamilyIndexCount, 0)
        require_equal(buffer_create_info.pQueueFamilyIndices, 0)

        get_buffer_memory_requirements = require(
            self.next_call_of("vkGetBufferMemoryRequirements"))
        require_not_equal(0, get_buffer_memory_requirements.int_Device)
        require_equal(written_buffer, get_buffer_memory_requirements.int_Buffer)
        require_not_equal(
            0, get_buffer_memory_requirements.hex_PMemoryRequirements)

        memory_requirements = VulkanStruct(
            architecture, MEMORY_REQUIREMENTS, get_write_offset_function(
                get_buffer_memory_requirements,
                get_buffer_memory_requirements.hex_PMemoryRequirements))

        require_equal(True, memory_requirements.size >= 1)
        require_not_equal(0, memory_requirements.alignment)
        require_not_equal(0, memory_requirements.memoryTypeBits)

        destroy_buffer = require(self.next_call_of("vkDestroyBuffer"))
        require_equal(destroy_buffer.int_Device,
            get_buffer_memory_requirements.int_Device)
        require_equal(written_buffer, destroy_buffer.int_Buffer)

        if self.not_device(device_properties, 0x5A400000,
                           gapit_test_framework.PIXEL_C):
            # Our second vkDestroySwapchain should have been called with
            # VK_NULL_HANDLE
            destroy_buffer = require(self.next_call_of("vkDestroyBuffer"))

            require_not_equal(0, destroy_buffer.int_Device)
            require_equal(0, destroy_buffer.int_Buffer)
