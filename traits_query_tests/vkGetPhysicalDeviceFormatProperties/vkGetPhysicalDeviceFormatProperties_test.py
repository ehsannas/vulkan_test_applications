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
from vulkan_constants import *
from struct_offsets import VulkanStruct, BOOL32, UINT32_T, POINTER, ARRAY


FORMAT_PROPERTIES = [
    ("linearTilingFeatures", UINT32_T),
    ("optimalTilingFeatures", UINT32_T),
    ("bufferFeatures", UINT32_T),
]


def GetPhysicalDevices(test, architecture):
    # first call to enumerate physical devices
    first_enumerate_physical_devices = require(test.next_call_of(
        "vkEnumeratePhysicalDevices"))
    require_equal(VK_SUCCESS, int(first_enumerate_physical_devices.return_val))
    require_not_equal(0, first_enumerate_physical_devices.int_Instance)
    require_not_equal(0,
                      first_enumerate_physical_devices.hex_PPhysicalDeviceCount)
    require_equal(0, first_enumerate_physical_devices.hex_PPhysicalDevices)

    num_phy_devices = little_endian_bytes_to_int(require(
        first_enumerate_physical_devices.get_write_data(
            first_enumerate_physical_devices.hex_PPhysicalDeviceCount,
            architecture.int_IntegerSize)))

    # second call to enumerate physical devices
    second_enumerate_physical_devices = require(test.next_call_of(
        "vkEnumeratePhysicalDevices"))
    require_equal(VK_SUCCESS, int(second_enumerate_physical_devices.return_val))
    require_not_equal(0, second_enumerate_physical_devices.int_Instance)
    require_not_equal(
        0, second_enumerate_physical_devices.hex_PPhysicalDeviceCount)
    require_not_equal(0, second_enumerate_physical_devices.hex_PPhysicalDevices)
    require_not_equal(0, num_phy_devices)
    PHYSICAL_DEVICES = [("physicalDevices", ARRAY, num_phy_devices, POINTER)]
    returned_physical_devices = VulkanStruct(
        architecture, PHYSICAL_DEVICES, get_write_offset_function(
            second_enumerate_physical_devices,
            second_enumerate_physical_devices.hex_PPhysicalDevices))
    return returned_physical_devices.physicalDevices


@gapit_test("vkGetPhysicalDeviceFormatProperties_test.apk")
class GetPhysicalDeviceFormatProperties(GapitTest):
    def expect(self):
        arch = self.architecture
        physical_devices = GetPhysicalDevices(self, arch)
        for pd in physical_devices:
            for fmt in ALL_VK_FORMATS:
                get_properties = require(self.next_call_of(
                    "vkGetPhysicalDeviceFormatProperties"))
                require_equal(pd, get_properties.int_PhysicalDevice)
                require_equal(fmt, get_properties.int_Format)
                require_not_equal(0, get_properties.hex_PFormatProperties)
                VulkanStruct(arch, FORMAT_PROPERTIES, get_write_offset_function(
                    get_properties, get_properties.hex_PFormatProperties))
