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


@gapit_test("PhysicalDeviceSurface_tests.apk")
class PhysicalDeviceSurfaceQueries(GapitTest):

    def expect(self):
        architecture = require(self.next_call_of("architecture"))

        enumerate_physical_devices = require(
            self.next_call_of("vkEnumeratePhysicalDevices"))

        num_physical_devices = little_endian_bytes_to_int(
            require(
                enumerate_physical_devices.get_write_data(
                    enumerate_physical_devices.hex_PPhysicalDeviceCount, 4)))
        for i in range(num_physical_devices):
            physical_device_properties = require(
                self.next_call_of("vkGetPhysicalDeviceProperties"))
            get_queue_family_properties = require(
                self.next_call_of("vkGetPhysicalDeviceQueueFamilyProperties"))
            num_queue_families = little_endian_bytes_to_int(
                require(
                    get_queue_family_properties.get_write_data(
                        get_queue_family_properties.
                        hex_PQueueFamilyPropertyCount, 4)))
            supports = False
            for j in range(num_queue_families):
                get_physical_device_surface_support = require(
                    self.next_call_of("vkGetPhysicalDeviceSurfaceSupportKHR"))
                supports_surface = little_endian_bytes_to_int(
                    require(
                        get_physical_device_surface_support.get_write_data(
                            get_physical_device_surface_support.hex_PSupported,
                            4)))
                supports = supports | (supports_surface > 0)

            if supports:
                get_physical_device_surface_capabilities = require(
                    self.next_call_of(
                        "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"))
                require_equal(
                    VK_SUCCESS,
                    int(get_physical_device_surface_capabilities.return_val))
                get_physical_device_surface_formats = require(
                    self.next_call_of("vkGetPhysicalDeviceSurfaceFormatsKHR"))

                get_physical_device_surface_formats_2 = require(
                    self.next_call_of("vkGetPhysicalDeviceSurfaceFormatsKHR"))

                get_physical_device_surface_formats_3 = require(
                    self.next_call_of("vkGetPhysicalDeviceSurfaceFormatsKHR"))

                max_surface_format_count = little_endian_bytes_to_int(
                    require(
                        get_physical_device_surface_formats.get_write_data(
                            get_physical_device_surface_formats.
                            hex_PSurfaceFormatCount, 4)))

                # 2 should request the same number as the return of 1
                require_equal(max_surface_format_count,
                              little_endian_bytes_to_int(
                                  require(
                                      get_physical_device_surface_formats_2.
                                      get_write_data(
                                          get_physical_device_surface_formats_2.
                                          hex_PSurfaceFormatCount, 4))))

                # https://b2.corp.google.com/issues/31490492 Broken on the
                # PixelC
                # 3 should request one fewer than 2
                if self.not_device(physical_device_properties, 0x5A400000,
                                   gapit_test_framework.PIXEL_C):
                    require_equal(
                        max_surface_format_count - 1,
                        little_endian_bytes_to_int(
                            require(
                                get_physical_device_surface_formats_3.
                                get_write_data(
                                    get_physical_device_surface_formats_3.
                                    hex_PSurfaceFormatCount, 4))))
                # 2 should have observed *PSurfaceFormatCount
                #                            * sizeof(VkSurfaceFormatKHR)
                # = *PSurfaceFormatCount * 4 * 2

                _ = require(
                    get_physical_device_surface_formats_2.get_write_data(
                        get_physical_device_surface_formats_2.
                        hex_PSurfaceFormats, max_surface_format_count * 8))
