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
from gapit_test_framework import GapitTest, get_write_offset_function
from gapit_test_framework import PIXEL_C, get_read_offset_function
import gapit_test_framework
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, POINTER, DEVICE_SIZE

MEMORY_REQUIREMENTS = [
    ("size", DEVICE_SIZE), ("alignment", DEVICE_SIZE),
    ("memoryTypeBits", UINT32_T)
]

MEMORY_ALLOCATE_INFO = [
    ("sType", UINT32_T), ("pNext", POINTER), ("allocationSize", DEVICE_SIZE),
    ("memoryTypeIndex", UINT32_T)
]


@gapit_test("ImageMemory_test.apk")
class GetImageMemoryRequirements(GapitTest):

    def expect(self):
        architecture = require(self.next_call_of("architecture"))
        get_image_memory_requirements = require(
            self.next_call_of("vkGetImageMemoryRequirements"))
        require_not_equal(0, get_image_memory_requirements.int_Device)
        require_not_equal(0, get_image_memory_requirements.int_Image)
        require_not_equal(0,
                          get_image_memory_requirements.hex_PMemoryRequirements)

        memory_requirements = VulkanStruct(
            architecture, MEMORY_REQUIREMENTS, get_write_offset_function(
                get_image_memory_requirements,
                get_image_memory_requirements.hex_PMemoryRequirements))

        require_equal(True, memory_requirements.size >= 1)
        require_not_equal(0, memory_requirements.alignment)
        require_not_equal(0, memory_requirements.memoryTypeBits)


@gapit_test("ImageMemory_test.apk")
class AllocateBindFreeMemory(GapitTest):

    def expect(self):
        architecture = require(self.next_call_of("architecture"))
        device_properties = require(
            self.next_call_of("vkGetPhysicalDeviceProperties"))

        get_image_memory_requirements = require(
            self.next_call_of("vkGetImageMemoryRequirements"))
        memory_requirements = VulkanStruct(
            architecture, MEMORY_REQUIREMENTS, get_write_offset_function(
                get_image_memory_requirements,
                get_image_memory_requirements.hex_PMemoryRequirements))

        allocate_memory = require(self.next_call_of("vkAllocateMemory"))
        require_not_equal(0, allocate_memory.int_Device)
        require_not_equal(0, allocate_memory.hex_PAllocateInfo)
        require_equal(0, allocate_memory.hex_PAllocator)
        require_not_equal(0, allocate_memory.hex_PMemory)

        allocate_memory_info = VulkanStruct(
            architecture, MEMORY_ALLOCATE_INFO,
            get_read_offset_function(allocate_memory,
                                     allocate_memory.hex_PAllocateInfo))

        require_equal(VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                      allocate_memory_info.sType)
        require_equal(0, allocate_memory_info.pNext)
        require_equal(memory_requirements.size,
                      allocate_memory_info.allocationSize)

        free_memory = require(self.next_call_of("vkFreeMemory"))
        require_not_equal(0, free_memory.int_Device)
        require_not_equal(0, free_memory.int_Memory)
        require_equal(0, free_memory.hex_PAllocator)

        if self.not_device(device_properties, 0x5A400000,
                           gapit_test_framework.PIXEL_C):
            free_memory = require(self.next_call_of("vkFreeMemory"))
            require_not_equal(0, free_memory.int_Device)
            require_equal(0, free_memory.int_Memory)
