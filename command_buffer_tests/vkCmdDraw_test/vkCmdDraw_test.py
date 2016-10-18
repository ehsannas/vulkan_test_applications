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


@gapit_test("vkCmdDraw_test.apk")
class DrawTriangle(GapitTest):

    def expect(self):
        draw = require(
            self.next_call_of("vkCmdDraw"))

        require_not_equal(0, draw.int_CommandBuffer)
        require_equal(3, draw.int_VertexCount)
        require_equal(1, draw.int_InstanceCount)
        require_equal(0, draw.int_FirstVertex)
        require_equal(0, draw.int_FirstInstance)
