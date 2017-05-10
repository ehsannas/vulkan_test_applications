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


@gapit_test("DrawCommands_test.apk")
class vkCmdDraw(GapitTest):

    def expect(self):
        draw = require(
            self.next_call_of("vkCmdDraw"))

        require_not_equal(0, draw.int_CommandBuffer)
        require_equal(3, draw.int_VertexCount)
        require_equal(1, draw.int_InstanceCount)
        require_equal(0, draw.int_FirstVertex)
        require_equal(0, draw.int_FirstInstance)


@gapit_test("DrawCommands_test.apk")
class vkCmdDrawIndexed(GapitTest):

    def expect(self):
        draw = require(
            self.next_call_of("vkCmdDrawIndexed"))

        require_not_equal(0, draw.int_CommandBuffer)
        require_equal(3, draw.int_IndexCount)
        require_equal(1, draw.int_InstanceCount)
        require_equal(0, draw.int_FirstIndex)
        require_equal(0, draw.int_VertexOffset)
        require_equal(0, draw.int_FirstInstance)


@gapit_test("DrawCommands_test.apk")
class vkCmdDrawIndirect(GapitTest):

    def expect(self):
        draw = require(
            self.next_call_of("vkCmdDrawIndirect"))

        require_not_equal(0, draw.int_CommandBuffer)
        require_not_equal(0, draw.int_Buffer)
        require_equal(0, draw.int_Offset)
        require_equal(1, draw.int_DrawCount)
        require_equal(0, draw.int_Stride)


@gapit_test("DrawCommands_test.apk")
class vkCmdDrawIndexedIndirect(GapitTest):

    def expect(self):
        draw = require(
            self.next_call_of("vkCmdDrawIndexedIndirect"))

        require_not_equal(0, draw.int_CommandBuffer)
        require_not_equal(0, draw.int_Buffer)
        require_equal(0, draw.int_Offset)
        require_equal(1, draw.int_DrawCount)
        require_equal(0, draw.int_Stride)