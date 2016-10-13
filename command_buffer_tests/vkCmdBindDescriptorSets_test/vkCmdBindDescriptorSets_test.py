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
from struct_offsets import VulkanStruct, UINT32_T, SIZE_T, POINTER
from struct_offsets import HANDLE, FLOAT, CHAR, ARRAY, DEVICE_SIZE
from vulkan_constants import *


@gapit_test("vkCmdBindDescriptorSets_test.apk")
class SingleDescriptorSet(GapitTest):

    def expect(self):
        bind_atom = require(
            self.next_call_of("vkCmdBindDescriptorSets"))

        require_not_equal(0, bind_atom.int_CommandBuffer)
        require_equal(VK_PIPELINE_BIND_POINT_GRAPHICS,
                      bind_atom.int_PipelineBindPoint)
        require_not_equal(0, bind_atom.hex_Layout)
        require_equal(0, bind_atom.int_FirstSet)
        require_equal(1, bind_atom.int_DescriptorSetCount)
        require_not_equal(0, bind_atom.hex_PDescriptorSets)
        require_equal(0, bind_atom.int_DynamicOffsetCount)
        require_equal(0, bind_atom.hex_PDynamicOffsets)

        descriptor_set = little_endian_bytes_to_int(require(
            bind_atom.get_read_data(
                bind_atom.hex_PDescriptorSets,
                NON_DISPATCHABLE_HANDLE_SIZE)))
        require_not_equal(0, descriptor_set)
