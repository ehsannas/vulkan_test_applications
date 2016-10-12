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

from gapit_test_framework import gapit_test, GapitTest
from gapit_test_framework import require, require_equal, require_not_equal
from gapit_test_framework import little_endian_bytes_to_int
from gapit_test_framework import PIXEL_C
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, HANDLE, POINTER, DEVICE_SIZE

WRITE_DESCRIPTOR_SET_ELEMENTS = [
    ("sType", UINT32_T),
    ("pNext", POINTER),
    ("dstSet", HANDLE),
    ("dstBinding", UINT32_T),
    ("dstArrayElement", UINT32_T),
    ("descriptorCount", UINT32_T),
    ("descriptorType", UINT32_T),
    ("pImageInfo", POINTER),
    ("pBufferInfo", POINTER),
    ("pTexelBufferView", POINTER),
]

DESCRIPTOR_BUFFER_INFO_ELEMENTS = [
    ("buffer", HANDLE),
    ("offset", DEVICE_SIZE),
    ("range", DEVICE_SIZE),
]


def get_buffer(test):
    """Returns the next buffer handle created by vkCreateBuffer."""
    create = require(test.next_call_of("vkCreateBuffer"))
    require_equal(VK_SUCCESS, int(create.return_val))
    buf = little_endian_bytes_to_int(
        require(create.get_write_data(
            create.hex_PBuffer, NON_DISPATCHABLE_HANDLE_SIZE)))
    require_not_equal(0, buf)
    return buf


def get_descriptor_set(test, index):
    """Returns the descriptor set handle created by the |index|th
    (starting from 1) vkAllocateDescriptorSet."""
    allocate = require(test.nth_call_of("vkAllocateDescriptorSets", index))
    require_equal(VK_SUCCESS, int(allocate.return_val))
    d_set = little_endian_bytes_to_int(
        require(allocate.get_write_data(
            allocate.hex_PDescriptorSets, NON_DISPATCHABLE_HANDLE_SIZE)))
    require_not_equal(0, d_set)
    return d_set


def get_write_descriptor_set(update_atom, architecture):
    """Returns a VulkanStruct representing the VkWriteDescriptorSet
    struct used in the given |update_atom| atom."""
    return VulkanStruct(
        architecture, WRITE_DESCRIPTOR_SET_ELEMENTS,
        lambda offset, size: little_endian_bytes_to_int(require(
            update_atom.get_read_data(
                update_atom.hex_PDescriptorWrites + offset, size))))


def get_buffer_info(update_atom, architecture, base, count):
    """Returns |count| VulkanStructs representing the VkDescriptorBufferInfo
    structs used in the VkWriteDescriptorSet parameter of the given
    |update_atom| atom."""
    buffer_info_size = 8 + 8 + 8  # handle, device size, device size

    infos = []
    for i in range(count):
        infos.append(VulkanStruct(
            architecture, DESCRIPTOR_BUFFER_INFO_ELEMENTS,
            lambda offset, size: little_endian_bytes_to_int(require(
                update_atom.get_read_data(
                    base + i * buffer_info_size + offset, size)))))
    return infos


@gapit_test("vkUpdateDescriptorSets_test.apk")
class ZeroWritesZeroCopy(GapitTest):
    def expect(self):
        """1. Zero writes and zero copies."""
        update_atom = require(self.nth_call_of("vkUpdateDescriptorSets", 1))
        require_equal(0, update_atom.DescriptorWriteCount)
        require_equal(0, update_atom.PDescriptorWrites)
        require_equal(0, update_atom.DescriptorCopyCount)
        require_equal(0, update_atom.PDescriptorCopies)


@gapit_test("vkUpdateDescriptorSets_test.apk")
class OneWriteZeroCopy(GapitTest):
    def expect(self):
        """2. One write and zero copies."""

        arch = require(self.next_call_of("architecture"))
        # Get the VkDescriptorSet handle returned from the driver.
        # This will also locate us to the proper position in the stream
        # so we can call next_call_of() for querying the other atoms.
        d_set = get_descriptor_set(self, 1)
        buf = get_buffer(self)

        update_atom = require(self.next_call_of("vkUpdateDescriptorSets"))
        require_equal(1, update_atom.DescriptorWriteCount)
        require_not_equal(0, update_atom.PDescriptorWrites)
        require_equal(0, update_atom.DescriptorCopyCount)
        require_equal(0, update_atom.PDescriptorCopies)

        # Check VkWriteDescriptorSet
        write = get_write_descriptor_set(update_atom, arch)
        require_equal(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, write.sType)
        require_equal(0, write.pNext)
        require_equal(d_set, write.dstSet)
        require_equal(0, write.dstBinding)
        require_equal(0, write.dstArrayElement)
        require_equal(2, write.descriptorCount)
        require_equal(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, write.descriptorType)
        require_equal(0, write.pImageInfo)
        require_not_equal(0, write.pBufferInfo)  # TODO
        require_equal(0, write.pTexelBufferView)

        # Check VkDescriptorBufferInfo
        bufinfo = get_buffer_info(update_atom, arch, write.pBufferInfo, 2)
        require_equal(buf, bufinfo[0].buffer)
        require_equal(000, bufinfo[0].offset)
        require_equal(512, bufinfo[0].range)
        require_equal(buf, bufinfo[1].buffer)
        require_equal(512, bufinfo[1].offset)
        require_equal(512, bufinfo[1].range)
