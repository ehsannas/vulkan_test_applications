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
#include "vulkan_wrapper/sub_device_objects.h"

namespace vulkan {
VkInstance CreateEmptyInstance(LibraryWrapper* _wrapper);

// Creates an instance with the swapchain and surface layers enabled.
VkInstance CreateDefaultInstance(LibraryWrapper* _wrapper);

containers::vector<VkPhysicalDevice> GetPhysicalDevices(
    containers::Allocator* allocator, VkInstance& instance);

// Gets all queue family properties for the given physical |device| from the
// given |instance|. Queue family properties will be returned in a vector
// allocated from the given |allocator|.
containers::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(
    containers::Allocator* allocator, VkInstance& instance,
    ::VkPhysicalDevice device);

// Returns the index for the first queue family with both graphics and compute
// capabilities for the given physical |device|. Returns the max unit32_t value
// if no such queue.
uint32_t GetGraphicsAndComputeQueueFamily(containers::Allocator* allocator,
                                          VkInstance& instance,
                                          ::VkPhysicalDevice device);

// Creates a device from the given |instance| with one queue. If
// |require_graphics_and_compute_queue| is true, the queue is of both graphics
// and compute capabilities.
VkDevice CreateDefaultDevice(containers::Allocator* allocator,
                             VkInstance& instance,
                             bool require_graphics_compute_queue = false);

VkCommandPool CreateDefaultCommandPool(containers::Allocator* allocator,
                                       VkDevice& device);
}
#endif  //  VULKAN_HELPERS_HELPER_FUNCTIONS_H_
