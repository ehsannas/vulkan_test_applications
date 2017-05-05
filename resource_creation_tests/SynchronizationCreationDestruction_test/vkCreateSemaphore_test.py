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
from vulkan_constants import *


@gapit_test("SynchronizationCreationDestruction_test.apk")
class SemaphoreCreateDestroyTest(GapitTest):

    def expect(self):
        architecture = self.architecture
        device_properties = require(
            self.next_call_of("vkGetPhysicalDeviceProperties"))

        create_semaphore = require(self.next_call_of("vkCreateSemaphore"))

        # Make sure the parameters are valid
        require_not_equal(create_semaphore.hex_PCreateInfo, 0)
        require_not_equal(create_semaphore.int_Device, 0)
        require_not_equal(create_semaphore.hex_PSemaphore, 0)
        require_equal(create_semaphore.hex_PAllocator, 0)

        create_info_structure_type_memory = require(
            create_semaphore.get_read_data(create_semaphore.hex_PCreateInfo,
                                           architecture.int_IntegerSize))
        create_info_pNext_memory = require(
            create_semaphore.get_read_data(create_semaphore.hex_PCreateInfo +
                                           architecture.int_PointerSize,
                                           architecture.int_PointerSize))
        create_info_flags_memory = require(
            create_semaphore.get_read_data(create_semaphore.hex_PCreateInfo + 2
                                           * architecture.int_PointerSize,
                                           architecture.int_IntegerSize))

        # The struct should look like
        # {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, 0, 0}
        require_equal(
            little_endian_bytes_to_int(create_info_structure_type_memory),
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO)
        require_equal(little_endian_bytes_to_int(create_info_pNext_memory), 0)
        require_equal(little_endian_bytes_to_int(create_info_flags_memory), 0)

        # pSemaphore is filled in by the call, should be a write observation
        returned_semaphore = require(
            create_semaphore.get_write_data(create_semaphore.hex_PSemaphore, 8))

        # We should have called destroy_semaphore with the same one
        destroy_semaphore = require(self.next_call_of("vkDestroySemaphore"))
        require_equal(
            little_endian_bytes_to_int(returned_semaphore),
            destroy_semaphore.int_Semaphore)
        if self.not_device(device_properties, 0x5A400000,
                           gapit_test_framework.PIXEL_C):
            # Our second destroy_semaphore should have been called with
            # VK_NULL_HANDLE
            destroy_semaphore_null = require(
                self.next_call_of("vkDestroySemaphore"))
            require_equal(0, destroy_semaphore_null.int_Semaphore)
