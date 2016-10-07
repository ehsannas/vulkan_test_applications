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

from gapit_test_framework import gapit_test, GapitTest, require, require_equal
from gapit_test_framework import require_not_equal, little_endian_bytes_to_int
from gapit_test_framework import PIXEL_C
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, HANDLE, POINTER

DESCRIPTOR_SET_ALLOCATE_INFO_ELEMENTS = [
    ("sType", UINT32_T),
    ("pNext", POINTER),
    ("descriptorPool", HANDLE),
    ("descriptorSetCount", UINT32_T),
    ("pSetLayouts", POINTER),
]


def check_alloc_descriptor_set(test, architecture):
    """Gets the next vkAllocateDescriptorSets call atom, and checks its return
    value and arguments. Also checks the returned pointer to descriptor set
    handles is not null. Returns the checked vkAllocateDescriptorSets atom,
    device and pointer to descriptor set handles. This method does not check
    the content of the VkDescriptorSetAllocateInfo struct used in the
    vkAllocateDescriptorSets call.
    """
    alloc_descriptor_set = require(
        test.next_call_of("vkAllocateDescriptorSets"))
    require_equal(VK_SUCCESS, int(alloc_descriptor_set.return_val))
    device = alloc_descriptor_set.int_Device
    require_not_equal(0, device)
    require_not_equal(0, alloc_descriptor_set.hex_PAllocateInfo)
    p_sets = alloc_descriptor_set.hex_PDescriptorSets
    require_not_equal(0, p_sets)
    return alloc_descriptor_set, device, p_sets


def check_free_descriptor_set(test, device, pool, sets):
    """Checks that the next vkFreeDescriptorSets command call atom is used to
    free the given descriptor |sets| from the given descriptor |pool| on the
    given |device|.
    """
    free_descriptor_set = require(
        test.next_call_of("vkFreeDescriptorSets"))
    require_equal(device, free_descriptor_set.int_Device)
    require_equal(pool, free_descriptor_set.int_DescriptorPool)
    require_equal(len(sets), free_descriptor_set.int_DescriptorSetCount)
    p_sets = free_descriptor_set.hex_PDescriptorSets
    for i, expected_set in enumerate(sets):
        actual_set = little_endian_bytes_to_int(
            require(free_descriptor_set.get_read_data(
                p_sets + NON_DISPATCHABLE_HANDLE_SIZE * i,
                NON_DISPATCHABLE_HANDLE_SIZE)))
        require_equal(expected_set, actual_set)


def get_descriptor_set_alloc_info(alloc_descriptor_set, architecture):
    """Returns a VulkanStruct representing the VkDescriptorSetAllocateInfo
    struct used in the given |alloc_descriptor_set| command."""
    return VulkanStruct(
        architecture, DESCRIPTOR_SET_ALLOCATE_INFO_ELEMENTS,
        lambda offset, size: little_endian_bytes_to_int(require(
            alloc_descriptor_set.get_read_data(
                alloc_descriptor_set.hex_PAllocateInfo + offset, size))))


def get_descriptor_pool(test, index):
    """Returns the handle of the |index|th (starting from 1) descriptor pool
    created from vkCreateDescriptorPool."""
    create = require(test.nth_call_of("vkCreateDescriptorPool", index))
    require_equal(VK_SUCCESS, int(create.return_val))
    pool = little_endian_bytes_to_int(
        require(create.get_write_data(
            create.hex_PDescriptorPool, NON_DISPATCHABLE_HANDLE_SIZE)))
    require_not_equal(0, pool)
    return pool


def get_descriptor_set_layout(test):
    """Returns the handle of the next descriptor set layout created from
    vkCreateDescriptorSetLayout."""
    create = require(test.next_call_of("vkCreateDescriptorSetLayout"))
    require_equal(VK_SUCCESS, int(create.return_val))
    layout = little_endian_bytes_to_int(
        require(create.get_write_data(
            create.hex_PSetLayout, NON_DISPATCHABLE_HANDLE_SIZE)))
    require_not_equal(0, layout)
    return layout


@gapit_test("AllocateFreeDescriptorSets_test.apk")
class OneDescriptorSet(GapitTest):
    def expect(self):
        """1. One descriptor set."""

        arch = require(self.next_call_of("architecture"))
        # Get the VkDescriptorPool handle returned from the driver.
        # This will also locate us to the proper position in the stream
        # so we can call next_call_of() for querying the other atoms.
        pool = get_descriptor_pool(self, 1)
        # Get the VkDescriptorSetLayout handle returned from the driver.
        set_layout = get_descriptor_set_layout(self)

        alloc_descriptor_set, device, p_sets = \
            check_alloc_descriptor_set(self, arch)
        info = get_descriptor_set_alloc_info(alloc_descriptor_set, arch)
        require_equal(info.sType,
                      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO)
        require_equal(0, info.pNext)
        require_equal(pool, info.descriptorPool)
        require_equal(1, info.descriptorSetCount)

        # Read the VkDescriptorSetLayout conveyed in the alloc info.
        layout = little_endian_bytes_to_int(
            require(alloc_descriptor_set.get_read_data(
                info.pSetLayouts, NON_DISPATCHABLE_HANDLE_SIZE)))
        require_equal(set_layout, layout)

        # Get the real VkDescriptorSet returned form the driver.
        actual_set = little_endian_bytes_to_int(
            require(alloc_descriptor_set.get_write_data(
                alloc_descriptor_set.hex_PDescriptorSets,
                NON_DISPATCHABLE_HANDLE_SIZE)))
        require_not_equal(0, actual_set)

        check_free_descriptor_set(self, device, pool, [actual_set])


@gapit_test("AllocateFreeDescriptorSets_test.apk")
class ThreeDescriptorSet(GapitTest):
    def expect(self):
        """2. Three descriptor sets."""

        arch = require(self.next_call_of("architecture"))
        # Get the VkDescriptorPool handle returned from the driver.
        # This will also locate us to the proper position in the stream
        # so we can call next_call_of() for querying the other atoms.
        pool = get_descriptor_pool(self, 2)
        # Get the VkDescriptorSetLayout handle returned from the driver.
        set_layout = get_descriptor_set_layout(self)

        alloc_descriptor_set, device, p_sets = \
            check_alloc_descriptor_set(self, arch)
        info = get_descriptor_set_alloc_info(alloc_descriptor_set, arch)
        require_equal(info.sType,
                      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO)
        require_equal(0, info.pNext)
        require_equal(pool, info.descriptorPool)
        require_equal(3, info.descriptorSetCount)

        for i in range(3):
            # Read the VkDescriptorSetLayout conveyed in the alloc info.
            layout = little_endian_bytes_to_int(
                require(alloc_descriptor_set.get_read_data(
                    info.pSetLayouts + NON_DISPATCHABLE_HANDLE_SIZE * i,
                    NON_DISPATCHABLE_HANDLE_SIZE)))
            require_equal(set_layout, layout)

        actual_set = []
        for i in range(3):
            # Get the real VkDescriptorSet returned form the driver.
            actual_set.append(little_endian_bytes_to_int(
                require(alloc_descriptor_set.get_write_data(
                    (alloc_descriptor_set.hex_PDescriptorSets +
                     NON_DISPATCHABLE_HANDLE_SIZE * i),
                    NON_DISPATCHABLE_HANDLE_SIZE))))
            require_not_equal(0, actual_set[-1])

        check_free_descriptor_set(self, device, pool, actual_set)


@gapit_test("AllocateFreeDescriptorSets_test.apk")
class FreeNullDescriptorSets(GapitTest):
    def expect(self):
        """3. Free null descriptor sets."""
        device_properties = require(
            self.next_call_of("vkGetPhysicalDeviceProperties"))

        if self.not_device(device_properties, 0x5A400000, PIXEL_C):
            free_descriptor_set = require(
                self.nth_call_of("vkFreeDescriptorSets", 3))
            p_sets = free_descriptor_set.hex_PDescriptorSets
            for i in range(2):
                actual_set = little_endian_bytes_to_int(
                    require(free_descriptor_set.get_read_data(
                        p_sets + NON_DISPATCHABLE_HANDLE_SIZE * i,
                        NON_DISPATCHABLE_HANDLE_SIZE)))
                require_equal(0, actual_set)
