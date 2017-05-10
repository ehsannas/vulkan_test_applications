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
from gapit_test_framework import get_read_offset_function
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, UINT64_T, ARRAY, POINTER, FLOAT

DEVICE_CREATE_INFO = [
    ("sType", UINT32_T),
    ("pNext", POINTER),
    ("flags", UINT32_T),
    ("queueCreateInfoCount", UINT32_T),
    ("pQueueCreateInfos", POINTER),
    ("enabledLayerCount", UINT32_T),
    ("ppEnabledLayerNames", POINTER),
    ("enabledExtensionCount", UINT32_T),
    ("ppEnabledExtensionNames", POINTER),
    ("pEnabledFeatures", POINTER),
]

DEVICE_QUEUE_CREATE_INFO = [
    ("sType", UINT32_T),
    ("pNext", POINTER),
    ("flags", UINT32_T),
    ("queueFamilyIndex", UINT32_T),
    ("queueCount", UINT32_T),
    ("pQueuePriorities", POINTER),
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

@gapit_test("vkCreateDevice_test.apk")
class ZeroQueueDevice(GapitTest):

    def expect(self):
        arch = self.architecture
        physical_device = GetPhysicalDevices(self, arch)[0]

        create_device = require(self.nth_call_of("vkCreateDevice", 1))
        require_equal(physical_device, create_device.int_PhysicalDevice)
        require_not_equal(0, create_device.hex_PCreateInfo)
        require_equal(0, create_device.hex_PAllocator)
        require_not_equal(0, create_device.hex_PDevice)
        require_equal(VK_SUCCESS, int(create_device.return_val))

        create_info = VulkanStruct(arch, DEVICE_CREATE_INFO,
                                   get_read_offset_function(
                                       create_device,
                                       create_device.hex_PCreateInfo))
        require_equal(VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, create_info.sType)
        # TODO(qining): Clear the pNext pointer assigned by the loader on
        # tracing device, then turn on the test
        #  require_equal(0, create_info.pNext)
        require_equal(0, create_info.flags)
        require_equal(0, create_info.queueCreateInfoCount)
        require_equal(0, create_info.pQueueCreateInfos)
        require_equal(0, create_info.enabledLayerCount)
        require_equal(0, create_info.ppEnabledLayerNames)
        require_equal(0, create_info.enabledExtensionCount)
        require_equal(0, create_info.ppEnabledExtensionNames)
        require_equal(0, create_info.pEnabledFeatures)

        DEVICE = [("device", POINTER)]
        device = VulkanStruct(arch, DEVICE, get_write_offset_function(
            create_device, create_device.hex_PDevice)).device
        require_not_equal(0, device)

        destroy_device = require(self.next_call_of("vkDestroyDevice"))
        require_equal(device, destroy_device.int_Device)
        require_equal(0, destroy_device.hex_PAllocator)


@gapit_test("vkCreateDevice_test.apk")
class OneQueueDevice(GapitTest):

    def expect(self):
        arch = self.architecture
        physical_device = GetPhysicalDevices(self, arch)[0]

        create_device = require(self.nth_call_of("vkCreateDevice", 2))
        require_equal(physical_device, create_device.int_PhysicalDevice)
        require_not_equal(0, create_device.hex_PCreateInfo)
        require_equal(0, create_device.hex_PAllocator)
        require_not_equal(0, create_device.hex_PDevice)

        create_info = VulkanStruct(arch, DEVICE_CREATE_INFO,
                                   get_read_offset_function(
                                       create_device,
                                       create_device.hex_PCreateInfo))
        require_equal(VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, create_info.sType)
        #  require_equal(0, create_info.pNext)
        require_equal(0, create_info.flags)
        require_equal(1, create_info.queueCreateInfoCount)
        require_not_equal(0, create_info.pQueueCreateInfos)
        require_equal(0, create_info.enabledLayerCount)
        require_equal(0, create_info.ppEnabledLayerNames)
        require_equal(0, create_info.enabledExtensionCount)
        require_equal(0, create_info.ppEnabledExtensionNames)
        require_equal(0, create_info.pEnabledFeatures)

        DEVICE = [("device", POINTER)]
        device = VulkanStruct(arch, DEVICE, get_write_offset_function(
            create_device, create_device.hex_PDevice)).device
        require_not_equal(0, device)

        queue_create_info = VulkanStruct(arch, DEVICE_QUEUE_CREATE_INFO,
                                         get_read_offset_function(create_device,
                                                                  create_info.pQueueCreateInfos))
        require_equal(VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, queue_create_info.sType)
        #  require_equal(0, queue_create_info.pNext)
        require_equal(0, queue_create_info.flags)
        require_equal(0, queue_create_info.queueFamilyIndex)
        require_equal(1, queue_create_info.queueCount)
        require_not_equal(0, queue_create_info.pQueuePriorities)

        PRIORITY = [("priority", FLOAT)]
        priority = VulkanStruct(arch, PRIORITY,
                                get_read_offset_function(create_device, queue_create_info.pQueuePriorities)).priority
        require_equal(1.0, priority)

        destroy_device = require(self.next_call_of("vkDestroyDevice"))
        require_equal(device, destroy_device.int_Device)
        require_equal(0, destroy_device.hex_PAllocator)
