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

from gapit_test_framework import gapit_test, GapitTest
from gapit_test_framework import require, require_equal, require_not_equal
from gapit_test_framework import get_write_offset_function
from vulkan_constants import *
from struct_offsets import VulkanStruct, UINT32_T, UINT64_T, ARRAY


@gapit_test("vkGetQueryPoolResults_test.apk")
class AllFourQueryResultsIn32BitWithNoFlag(GapitTest):

    def expect(self):
        num_queries = 4
        result_width = 4

        architecture = self.architecture
        get_query_result = require(self.nth_call_of("vkGetQueryPoolResults", 1))

        require_not_equal(0, get_query_result.int_Device)
        require_not_equal(0, get_query_result.int_QueryPool)
        require_equal(0, get_query_result.int_FirstQuery)
        require_equal(num_queries, get_query_result.int_QueryCount)
        require_equal(num_queries * result_width, get_query_result.int_DataSize)
        require_not_equal(0, get_query_result.hex_PData)
        require_equal(result_width, get_query_result.int_Stride)
        require_equal(0, get_query_result.int_Flags)
        require_equal(VK_SUCCESS, int(get_query_result.return_val))

        result_data = VulkanStruct(
            architecture, [("data", ARRAY, num_queries, UINT32_T)],
            get_write_offset_function(get_query_result,
                                      get_query_result.hex_PData))
        require_equal([0 for i in range(num_queries)], result_data.data)


@gapit_test("vkGetQueryPoolResults_test.apk")
class FifthToEighthQueryResultsIn64BitWithWaitBit(GapitTest):

    def expect(self):
        num_queries = 8
        get_result_num_queries = 4
        result_width = 8

        architecture = self.architecture
        get_query_result = require(self.nth_call_of("vkGetQueryPoolResults", 2))

        require_not_equal(0, get_query_result.int_Device)
        require_not_equal(0, get_query_result.int_QueryPool)
        require_equal(num_queries - get_result_num_queries,
                      get_query_result.int_FirstQuery)
        require_equal(get_result_num_queries, get_query_result.int_QueryCount)
        require_equal(result_width * get_result_num_queries,
                      get_query_result.int_DataSize)
        require_not_equal(0, get_query_result.hex_PData)
        require_equal(result_width, get_query_result.int_Stride)
        require_equal(VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT,
                      get_query_result.int_Flags)
        require_equal(VK_SUCCESS, int(get_query_result.return_val))

        result_data = VulkanStruct(
            architecture, [("data", ARRAY, get_result_num_queries, UINT64_T)],
            get_write_offset_function(get_query_result,
                                      get_query_result.hex_PData))
        require_equal([0 for i in range(get_result_num_queries)],
                      result_data.data)


@gapit_test("vkGetQueryPoolResults_test.apk")
class AllFourQueryResultsIn32BitAnd12StrideWithPartialAndAvailabilityBit(
        GapitTest):

    def expect(self):
        num_queries = 4
        stride = 12

        architecture = self.architecture
        get_query_result = require(self.nth_call_of("vkGetQueryPoolResults", 3))

        require_not_equal(0, get_query_result.int_Device)
        require_not_equal(0, get_query_result.int_QueryPool)
        require_equal(0, get_query_result.int_FirstQuery)
        require_equal(num_queries, get_query_result.int_QueryCount)
        require_equal(num_queries * stride, get_query_result.int_DataSize)
        require_not_equal(0, get_query_result.hex_PData)
        require_equal(stride, get_query_result.int_Stride)
        require_equal(VK_QUERY_RESULT_PARTIAL_BIT
                      | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT,
                      get_query_result.int_Flags)
        require_equal(VK_SUCCESS, int(get_query_result.return_val))

        result_data = VulkanStruct(
            architecture, [("data", ARRAY, num_queries * stride / 4, UINT32_T)],
            get_write_offset_function(get_query_result,
                                      get_query_result.hex_PData))
        expected_data = [0, 1, 0xFFFFFFFF, 0, 1, 0xFFFFFFFF, 0, 1, 0xFFFFFFFF,
                         0, 1, 0xFFFFFFFF]
        require_equal(expected_data, result_data.data)
