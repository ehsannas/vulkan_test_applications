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
from struct_offsets import *
from vulkan_constants import *

FENCE_CREATE_INFO = [
    ("sType", UINT32_T),
    ("pNext", POINTER),
    ("flags", UINT32_T)
]


@gapit_test("Fence_test.apk")
class CreateDestroyWaitTest(GapitTest):

    def expect(self):

        architecture = self.architecture
        create_fence = require(self.nth_call_of("vkCreateFence", 1))
        wait_for_fences = require(self.next_call_of("vkWaitForFences"))
        reset_fences = require(self.next_call_of("vkResetFences"))
        destroy_fence = require(self.next_call_of("vkDestroyFence"))

        require_not_equal(0, create_fence.int_Device)
        require_not_equal(0, create_fence.hex_PCreateInfo)
        require_equal(0, create_fence.hex_PAllocator)
        require_not_equal(0, create_fence.hex_PFence)

        create_info = VulkanStruct(architecture,
                                   FENCE_CREATE_INFO, get_read_offset_function(
                                   create_fence, create_fence.hex_PCreateInfo))

        require_equal(VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, create_info.sType)
        require_equal(0, create_info.pNext)
        require_equal(0, create_info.flags)

        returned_fence = little_endian_bytes_to_int(
            require(
                create_fence.get_write_data(
                    create_fence.hex_PFence, NON_DISPATCHABLE_HANDLE_SIZE)))
        require_not_equal(0, returned_fence)

        require_equal(create_fence.int_Device, wait_for_fences.int_Device)
        require_equal(1, wait_for_fences.int_FenceCount)
        require_not_equal(1, wait_for_fences.hex_PFences)
        require_equal(0, wait_for_fences.int_WaitAll)
        require_equal(10000, wait_for_fences.int_Timeout)

        waited_for_fence = little_endian_bytes_to_int(
            require(
                wait_for_fences.get_read_data(
                    wait_for_fences.hex_PFences, NON_DISPATCHABLE_HANDLE_SIZE
                )))
        require_equal(waited_for_fence, returned_fence)

        require_equal(create_fence.int_Device, reset_fences.int_Device)
        require_equal(1, reset_fences.int_FenceCount)
        require_equal(create_fence.int_Device, reset_fences.int_Device)

        reset_fence = little_endian_bytes_to_int(
            require(
                reset_fences.get_read_data(
                    wait_for_fences.hex_PFences, NON_DISPATCHABLE_HANDLE_SIZE
                )))
        require_equal(returned_fence, reset_fence)

        require_equal(create_fence.int_Device, destroy_fence.int_Device)
        require_equal(returned_fence, destroy_fence.int_Fence)
        require_equal(0, destroy_fence.hex_PAllocator)


@gapit_test("Fence_test.apk")
class DestroyEmpty(GapitTest):

    def expect(self):

        device_properties = require(
            self.next_call_of("vkGetPhysicalDeviceProperties"))

        if self.not_device(device_properties, 0x5A400000,
                           gapit_test_framework.PIXEL_C):
            destroy_fence = require(self.nth_call_of("vkDestroyFence", 2))
            require_not_equal(0, destroy_fence.int_Device)
            require_equal(0, destroy_fence.int_Fence)
            require_equal(0, destroy_fence.hex_PAllocator)
