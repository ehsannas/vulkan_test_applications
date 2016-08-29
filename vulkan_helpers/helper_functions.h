/* Copyright 2016 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VULKAN_HELPERS_HELPER_FUNCTIONS_H_
#define VULKAN_HELPERS_HELPER_FUNCTIONS_H_

#include "support/containers/vector.h"
#include "vulkan_wrapper/device_wrapper.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"

namespace vulkan {
VkInstance CreateEmptyInstance(LibraryWrapper* _wrapper);
containers::vector<VkPhysicalDevice> GetPhysicalDevices(
    containers::Allocator* allocator, VkInstance& instance);
VkDevice CreateDefaultDevice(containers::Allocator* allocator,
                             VkInstance& instance);
}
#endif  //  VULKAN_HELPERS_HELPER_FUNCTIONS_H_
