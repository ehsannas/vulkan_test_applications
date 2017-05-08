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
from gapit_test_framework import require_not_equal, GapitTest, PIXEL_C
from gapit_test_framework import get_read_offset_function, get_write_offset_function
from gapit_test_framework import GapidUnsupportedException
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, POINTER, HANDLE, DEVICE_SIZE
from struct_offsets import ARRAY, CHAR

QUERY_POOL_CREATE_INFO = [
    ("sType", UINT32_T),
    ("pNext", POINTER),
    ("flags", UINT32_T),
    ("queryType", UINT32_T),
    ("queryCount", UINT32_T),
    ("pipelineStatistics", UINT32_T),
]

QUERY_POOL = [("handle", HANDLE)]


@gapit_test("CreateDestroyQueryPool_test.apk")
class OneQueryOcclusionQueryPool(GapitTest):

    def expect(self):
        """1. Expects a query pool to be created with queryCount: 1 and
        queryType: VK_QUERY_TYPE_OCCLUSION."""

        architecture = self.architecture
        device_properties = require(self.next_call_of(
            "vkGetPhysicalDeviceProperties"))

        create_query_pool = require(self.nth_call_of("vkCreateQueryPool", 1))
        device = create_query_pool.int_Device
        require_not_equal(0, device)
        require_not_equal(0, create_query_pool.hex_PCreateInfo)
        require_equal(0, create_query_pool.hex_PAllocator)
        require_not_equal(0, create_query_pool.hex_PQueryPool)
        require_equal(VK_SUCCESS, int(create_query_pool.return_val))

        create_info = VulkanStruct(
            architecture, QUERY_POOL_CREATE_INFO, get_read_offset_function(
                create_query_pool, create_query_pool.hex_PCreateInfo))
        require_equal(VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                      create_info.sType)
        require_equal(0, create_info.pNext)
        require_equal(0, create_info.flags)
        require_equal(VK_QUERY_TYPE_OCCLUSION, create_info.queryType)
        require_equal(1, create_info.queryCount)
        require_equal(0, create_info.pipelineStatistics)

        view = VulkanStruct(
            architecture, QUERY_POOL, get_write_offset_function(
                create_query_pool, create_query_pool.hex_PQueryPool))
        require_not_equal(0, view.handle)

        destroy_query_pool = require(self.next_call_of("vkDestroyQueryPool"))
        require_equal(device, destroy_query_pool.int_Device)
        require_equal(view.handle, destroy_query_pool.int_QueryPool)

        if self.not_device(device_properties, 0x5A400000, PIXEL_C):
            destroy_null_query_pool = require(self.next_call_of(
                "vkDestoryQueryPool"))
            require_equal(device, destroy_query_pool.int_Device)
            require_equal(0, destroy_null_query_pool.int_QueryPool)


@gapit_test("CreateDestroyQueryPool_test.apk")
class SevenQueriesTimeStampQueryPool(GapitTest):

    def expect(self):
        """2. Expects a query pool to be created with queryCount: 7 and
        queryType: VK_QUERY_TYPE_TIMESTAMP."""

        architecture = self.architecture
        device_properties = require(self.next_call_of(
            "vkGetPhysicalDeviceProperties"))

        create_query_pool = require(self.nth_call_of("vkCreateQueryPool", 2))
        device = create_query_pool.int_Device
        require_not_equal(0, device)
        require_not_equal(0, create_query_pool.hex_PCreateInfo)
        require_equal(0, create_query_pool.hex_PAllocator)
        require_not_equal(0, create_query_pool.hex_PQueryPool)
        require_equal(VK_SUCCESS, int(create_query_pool.return_val))

        create_info = VulkanStruct(
            architecture, QUERY_POOL_CREATE_INFO, get_read_offset_function(
                create_query_pool, create_query_pool.hex_PCreateInfo))
        require_equal(VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                      create_info.sType)
        require_equal(0, create_info.pNext)
        require_equal(0, create_info.flags)
        require_equal(VK_QUERY_TYPE_TIMESTAMP, create_info.queryType)
        require_equal(7, create_info.queryCount)
        require_equal(0, create_info.pipelineStatistics)

        view = VulkanStruct(
            architecture, QUERY_POOL, get_write_offset_function(
                create_query_pool, create_query_pool.hex_PQueryPool))
        require_not_equal(0, view.handle)

        destroy_query_pool = require(self.next_call_of("vkDestroyQueryPool"))
        require_equal(device, destroy_query_pool.int_Device)
        require_equal(view.handle, destroy_query_pool.int_QueryPool)

        if self.not_device(device_properties, 0x5A400000, PIXEL_C):
            destroy_null_query_pool = require(self.next_call_of(
                "vkDestoryQueryPool"))
            require_equal(device, destroy_query_pool.int_Device)
            require_equal(0, destroy_null_query_pool.int_QueryPool)


@gapit_test("CreateDestroyQueryPool_test.apk")
class FourQueriesPipelineStatisticsQueryPool(GapitTest):

    def expect(self):
        """3. Expects a query pool to be created with queryCount: 4 and
        queryType: VK_QUERY_TYPE_PIPELINE_STATISTICS and pipelineStatistics:
        VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT."""

        architecture = self.architecture
        device_properties = require(self.next_call_of(
            "vkGetPhysicalDeviceProperties"))

        create_device = self.nth_call_of("vkCreateDevice", 3)
        if create_device[0] is None:
            raise GapidUnsupportedException(
                "physical device feature: pipelineStatistics not supported")

        create_query_pool = require(self.next_call_of("vkCreateQueryPool"))
        device = create_query_pool.int_Device
        require_not_equal(0, device)
        require_not_equal(0, create_query_pool.hex_PCreateInfo)
        require_equal(0, create_query_pool.hex_PAllocator)
        require_not_equal(0, create_query_pool.hex_PQueryPool)
        require_equal(VK_SUCCESS, int(create_query_pool.return_val))

        create_info = VulkanStruct(
            architecture, QUERY_POOL_CREATE_INFO, get_read_offset_function(
                create_query_pool, create_query_pool.hex_PCreateInfo))
        require_equal(VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
                      create_info.sType)
        require_equal(0, create_info.pNext)
        require_equal(0, create_info.flags)
        require_equal(VK_QUERY_TYPE_PIPELINE_STATISTICS, create_info.queryType)
        require_equal(4, create_info.queryCount)
        require_equal(VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT,
                      create_info.pipelineStatistics)

        view = VulkanStruct(
            architecture, QUERY_POOL, get_write_offset_function(
                create_query_pool, create_query_pool.hex_PQueryPool))
        require_not_equal(0, view.handle)

        destroy_query_pool = require(self.next_call_of("vkDestroyQueryPool"))
        require_equal(device, destroy_query_pool.int_Device)
        require_equal(view.handle, destroy_query_pool.int_QueryPool)

        if self.not_device(device_properties, 0x5A400000, PIXEL_C):
            destroy_null_query_pool = require(self.next_call_of(
                "vkDestoryQueryPool"))
            require_equal(device, destroy_query_pool.int_Device)
            require_equal(0, destroy_null_query_pool.int_QueryPool)
