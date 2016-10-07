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
#include "vulkan_wrapper/queue_wrapper.h"
#include "vulkan_wrapper/sub_objects.h"
#include "vulkan_wrapper/swapchain.h"

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
// The |allocator| must continue to exist until the device is destroyed.
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

// Creates a 2D color-attachment R8G8B8A8 unorm format image with the specified
// width and height. The image is not multi-sampled and is in exclusive sharing
// mode. Its mipLevels and arrayLayers are set to 1, its image tiling is set to
// VK_IMAGE_TILING_OPTIMAL and its initialLayout is set to
// VK_IMAGE_LAYOUT_UNDEFINED.
VkImage CreateDefault2DColorImage(VkDevice* device, uint32_t width,
                                  uint32_t height);

// Creates a default sampler with normalized coordinates. magFilter, minFilter,
// and mipmap are all using nearest mode. Addressing modes for U, V, and W
// coordinates are all clamp-to-edge. mipLodBias, minLod, and maxLod are all 0.
// anisotropy and compare is disabled.
VkSampler CreateDefaultSampler(VkDevice* device);

// Creates a DescriptorSetLayout with the given set of layouts.
VkDescriptorSetLayout CreateDescriptorSetLayout(
    containers::Allocator* allocator, VkDevice* device,
    std::initializer_list<VkDescriptorSetLayoutBinding> bindings);

// Returns the first queue from the given family.
VkQueue inline GetQueue(VkDevice* device, uint32_t queue_family_index) {
  ::VkQueue queue;
  (*device)->vkGetDeviceQueue(*device, queue_family_index, 0, &queue);
  return VkQueue(queue, device);
}

// Given a bitmask of required_index_bits and a bitmask of
// VkMemoryPropertyFlags, return the first memory index
// from the given device that supports the required_property_flags.
// Will assert if one could not be found.
uint32_t inline GetMemoryIndex(VkDevice* device, logging::Logger* log,
                               uint32_t required_index_bits,
                               VkMemoryPropertyFlags required_property_flags) {
  const VkPhysicalDeviceMemoryProperties& properties =
      device->physical_device_memory_properties();
  LOG_ASSERT(<=, log, properties.memoryTypeCount, 32);
  uint32_t memory_index = 0;
  for (; memory_index < properties.memoryTypeCount; ++memory_index) {
    if (!(required_index_bits & (1 << memory_index))) {
      continue;
    }

    if (!(properties.memoryTypes[memory_index].propertyFlags &
          required_property_flags)) {
      continue;
    }
    break;
  }
  LOG_ASSERT(!=, log, memory_index, properties.memoryTypeCount);
  return memory_index;
}

// Runs the given call once with a nullptr value, and gets the numerical result.
// Resizes the given array, and runs the call again to fill the array.
// Asserts that the function call succeeded.
template <typename Function, typename... Args, typename ContainedType>
void LoadContainer(logging::Logger* log, Function& fn,
                   containers::vector<ContainedType>* ret_val, Args&... args) {
  uint32_t num_values = 0;
  LOG_ASSERT(==, log, (fn)(args..., &num_values, nullptr), VK_SUCCESS);
  ret_val->resize(num_values);
  LOG_ASSERT(==, log, (fn)(args..., &num_values, ret_val->data()), VK_SUCCESS);
}
}  // namespace vulkan

#endif  //  VULKAN_HELPERS_HELPER_FUNCTIONS_H_
