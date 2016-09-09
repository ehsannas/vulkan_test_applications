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

from gapit_test_framework import gapit_test, require, require_not, require_equal
from gapit_test_framework import require_not_equal, little_endian_bytes_to_int
from gapit_test_framework import GapitTest
from vulkan_constants import *


@gapit_test("vkCreateInstance_test.apk")
class NullObjectTest(GapitTest):

    def expect(self):
        """Expect that the applicationInfoPointer is null for the first
         vkCreateInstance"""
        architecture = require(self.next_call_of("architecture"))

        create_instance = require(self.next_call_of("vkCreateInstance"))
        require_not_equal(create_instance.hex_PCreateInfo, 0)

        create_info_memory = require(
            create_instance.get_read_data(create_instance.hex_PCreateInfo,
                                          architecture.int_IntegerSize))
        require_equal(
            little_endian_bytes_to_int(create_info_memory),
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO)

        applicationInfoPointer = require(
            create_instance.get_read_data(create_instance.hex_PCreateInfo +
                                          architecture.int_PointerSize * 3,
                                          architecture.int_PointerSize))
        require_equal(little_endian_bytes_to_int(applicationInfoPointer), 0)
