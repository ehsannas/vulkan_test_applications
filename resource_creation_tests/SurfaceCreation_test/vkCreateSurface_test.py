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
from vulkan_constants import *


@gapit_test("SurfaceCreation_test.apk")
class SurfaceCreateTest(GapitTest):

    def expect(self):
        """Expect that the applicationInfoPointer is null for the first
         vkCreateInstance"""
        architecture = require(self.next_call_of("architecture"))
        create_surface = require(self.next_call_of("vkCreateAndroidSurfaceKHR"))

        # Make sure the parameters are valid
        require_not_equal(create_surface.int_Instance, 0)
        require_not_equal(create_surface.hex_PCreateInfo, 0)
        require_equal(create_surface.hex_PAllocator, 0)
        require_not_equal(create_surface.hex_PSurface, 0)

        create_surface_structure_type_memory = require(
            create_surface.get_read_data(create_surface.hex_PCreateInfo,
                                         architecture.int_IntegerSize))
        create_surface_pNext_memory = require(
            create_surface.get_read_data(create_surface.hex_PCreateInfo +
                                         architecture.int_PointerSize,
                                         architecture.int_PointerSize))
        create_surface_flags_memory = require(
            create_surface.get_read_data(create_surface.hex_PCreateInfo + 2 *
                                         architecture.int_PointerSize,
                                         architecture.int_IntegerSize))
        create_surface_native_window_memory = require(
            create_surface.get_read_data(create_surface.hex_PCreateInfo + 3 *
                                         architecture.int_PointerSize,
                                         architecture.int_PointerSize))

        # The struct should look like
        # {VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR, 0, 0, window}
        require_equal(
            little_endian_bytes_to_int(create_surface_structure_type_memory),
            VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR)
        require_equal(
            little_endian_bytes_to_int(create_surface_pNext_memory), 0)
        require_equal(
            little_endian_bytes_to_int(create_surface_flags_memory), 0)
        require_not_equal(
            little_endian_bytes_to_int(create_surface_native_window_memory), 0)

        # pSemaphore is filled in by the call, should be a write observation
        surface = require(
            create_surface.get_write_data(create_surface.hex_PSurface,
                                          NON_DISPATCHABLE_HANDLE_SIZE))

        # We should have called destroy_semaphore with the same one
        destroy_surface = require(self.next_call_of("vkDestroySurfaceKHR"))
        require_equal(
            little_endian_bytes_to_int(surface), destroy_surface.int_Surface)

        destroy_surface_2 = require(self.next_call_of("vkDestroySurfaceKHR"))
        require_equal(0, destroy_surface.int_Surface)
