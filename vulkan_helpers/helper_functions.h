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
#include "support/entry/entry.h"
#include "vulkan_wrapper/command_buffer_wrapper.h"
#include "vulkan_wrapper/device_wrapper.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"
#include "vulkan_wrapper/sub_objects.h"

namespace vulkan {
// Create an empty instance. Vulkan functions that are resolved by the created
// instance will be stored in the space allocated by the given |allocator|. The
// |allocator| must continue to exist until the instance is destroied.
VkInstance CreateEmptyInstance(containers::Allocator* allocator,
                               LibraryWrapper* _wrapper);

// Creates an instance with the swapchain and surface layers enabled. Vulkan
// functions that are resolved through the created instance will be stored in
// the space allocated by the given |allocator|. The |allocator| must continue
// to exist until the instance is destroied.
VkInstance CreateDefaultInstance(containers::Allocator* allocator,
                                 LibraryWrapper* _wrapper);

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
// and compute capabilities. Vulkan functions that are resolved through the
// create device will be stored in the space allocated by the given |allocator|.
// The |allocator| must continue to exist until the device is destroied.
VkDevice CreateDefaultDevice(containers::Allocator* allocator,
                             VkInstance& instance,
                             bool require_graphics_compute_queue = false);

// Creates a command pool with VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
// set.
VkCommandPool CreateDefaultCommandPool(containers::Allocator* allocator,
                                       VkDevice& device);

// Creates a surface to render into the the default window
// provided in entry_data.
VkSurfaceKHR CreateDefaultSurface(VkInstance* instance,
                                  const entry::entry_data* entry_data);

// Creates a device capable of presenting to the given surface.
// Returns the queue indices for the present and graphics queues.
// Note: They may be the same or different.
VkDevice CreateDeviceForSwapchain(containers::Allocator* allocator,
                                  VkInstance* instance, VkSurfaceKHR* surface,
                                  uint32_t* present_queue_index,
                                  uint32_t* graphics_queue_index);

// Creates a default command buffer from the given command pool and the device
// with primary level.
VkCommandBuffer CreateDefaultCommandBuffer(VkCommandPool* pool,
                                           VkDevice* device);

// Creates a swapchain with a default layout and number of images.
// It will be able to be rendered to from graphics_queue_index,
// and it will be presentable on present_queue_index.
VkSwapchainKHR CreateDefaultSwapchain(VkInstance* instance, VkDevice* device,
                                      VkSurfaceKHR* surface,
                                      containers::Allocator* allocator,
                                      uint32_t present_queue_index,
                                      uint32_t graphics_queue_index);

// Returns a uint32_t with only the lowest bit set.
uint32_t inline GetLSB(uint32_t val) { return ((val - 1) ^ val) & val; }
}

#endif  //  VULKAN_HELPERS_HELPER_FUNCTIONS_H_
