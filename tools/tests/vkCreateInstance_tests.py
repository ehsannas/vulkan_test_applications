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

from gapit_test_framework import gapit_test, require, require_not
from gapit_test_framework import GapitTest


@gapit_test("vkCreateInstance_test.apk")
class NullObjectTest(GapitTest):

    def expect(self):
        """Expect that there is a call with a null argument"""
        require(self.next_call_of("vkCreateInstance"))
        require(self.next_call_of("vkDestroyInstance"))
        # In this test there should be 2 more vkCreateInstance calls
        require(self.nth_call_of("vkCreateInstance", 2))
        # There should be no more calls to vkCreateInstance
        require_not(self.next_call_of("vkCreateInstance"))
