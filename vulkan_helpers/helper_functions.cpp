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

#include "vulkan_helpers/helper_functions.h"

#include "support/containers/vector.h"
#include "support/log/log.h"

namespace vulkan {
VkInstance CreateEmptyInstance(LibraryWrapper* wrapper) {
  // Test a non-nullptr pApplicationInfo
  VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             nullptr,
                             "TestApplication",
                             1,
                             "Engine",
                             0,
                             VK_MAKE_VERSION(1, 0, 0)};

  VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                            nullptr,
                            0,
                            &app_info,
                            0,
                            nullptr,
                            0,
                            nullptr};

  ::VkInstance raw_instance;
  LOG_ASSERT(==, wrapper->GetLogger(),
             wrapper->vkCreateInstance(&info, nullptr, &raw_instance),
             VK_SUCCESS);
  // vulkan::VkInstance will handle destroying the instance
  return vulkan::VkInstance(raw_instance, nullptr, wrapper);
}

VkInstance CreateDefaultInstance(LibraryWrapper* wrapper) {
  // Similar to Empty Instance, but turns on the platform specific
  // swapchian functions.
  // Test a non-nullptr pApplicationInfo
  VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             nullptr,
                             "TestApplication",
                             1,
                             "Engine",
                             0,
                             VK_MAKE_VERSION(1, 0, 0)};

  const char* extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined __ANDROID__
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined __linux__
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined __WIN32__
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
  };

  wrapper->GetLogger()->LogInfo("Enabled Extensions: ");
  for (auto& extension : extensions) {
    wrapper->GetLogger()->LogInfo("    ", extension);
  }

  VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                            nullptr,
                            0,
                            &app_info,
                            0,
                            nullptr,
                            sizeof(extensions) / sizeof(extensions[0]),
                            extensions};

  ::VkInstance raw_instance;
  LOG_ASSERT(==, wrapper->GetLogger(),
             wrapper->vkCreateInstance(&info, nullptr, &raw_instance),
             VK_SUCCESS);
  // vulkan::VkInstance will handle destroying the instance
  return vulkan::VkInstance(raw_instance, nullptr, wrapper);
}

containers::vector<VkPhysicalDevice> GetPhysicalDevices(
    containers::Allocator* allocator, VkInstance& instance) {
  uint32_t device_count = 0;
  instance.vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  containers::vector<VkPhysicalDevice> physical_devices(device_count,
                                                        allocator);
  LOG_ASSERT(==, instance.GetLogger(),
             instance.vkEnumeratePhysicalDevices(instance, &device_count,
                                                 physical_devices.data()),
             VK_SUCCESS);

  return std::move(physical_devices);
}

containers::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(
    containers::Allocator* allocator, VkInstance& instance,
    ::VkPhysicalDevice device) {
  uint32_t count = 0;
  instance.vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

  LOG_ASSERT(>, instance.GetLogger(), count, 0u);
  containers::vector<VkQueueFamilyProperties> properties(allocator);
  properties.resize(count);
  instance.vkGetPhysicalDeviceQueueFamilyProperties(device, &count,
                                                    properties.data());
  return std::move(properties);
}

inline bool HasGraphicsAndComputeQueue(
    const VkQueueFamilyProperties& property) {
  return property.queueCount > 0 &&
         (property.queueFlags & VK_QUEUE_GRAPHICS_BIT & VK_QUEUE_COMPUTE_BIT);
}

uint32_t GetGraphicsAndComputeQueueFamily(containers::Allocator* allocator,
                                          VkInstance& instance,
                                          ::VkPhysicalDevice device) {
  uint32_t queue_family_index = ~0u;

  auto properties = GetQueueFamilyProperties(allocator, instance, device);
  // We use the max uint32_t value to mean failure.
  LOG_ASSERT(!=, instance.GetLogger(), properties.size(), ~0u);

  for (uint32_t i = 0; i < properties.size(); ++i) {
    if (HasGraphicsAndComputeQueue(properties[i])) break;
  }
  return queue_family_index;
}

VkDevice CreateDefaultDevice(containers::Allocator* allocator,
                             VkInstance& instance,
                             bool require_graphics_compute_queue) {
  containers::vector<VkPhysicalDevice> physical_devices =
      GetPhysicalDevices(allocator, instance);
  float priority = 1.f;

  VkPhysicalDevice physical_device = physical_devices.front();

  VkPhysicalDeviceProperties properties;
  instance.vkGetPhysicalDeviceProperties(physical_device, &properties);

  const uint32_t queue_family_index =
      require_graphics_compute_queue ? GetGraphicsAndComputeQueueFamily(
                                           allocator, instance, physical_device)
                                     : 0;
  LOG_ASSERT(!=, instance.GetLogger(), queue_family_index, ~0u);

  VkDeviceQueueCreateInfo queue_info{
      /* sType = */ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      /* pNext = */ nullptr,
      /* flags = */ 0,
      /* queueFamilyIndex = */ queue_family_index,
      /* queueCount */ 1,
      /* pQueuePriorities = */ &priority};

  VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                          nullptr,
                          0,
                          1,
                          &queue_info,
                          0,
                          nullptr,
                          0,
                          nullptr,
                          nullptr};

  ::VkDevice raw_device;
  LOG_ASSERT(
      ==, instance.GetLogger(),
      instance.vkCreateDevice(physical_device, &info, nullptr, &raw_device),
      VK_SUCCESS);
  return vulkan::VkDevice(raw_device, nullptr, &instance, &properties);
}

VkCommandPool CreateDefaultCommandPool(containers::Allocator* allocator,
                                       VkDevice& device) {
  VkCommandPoolCreateInfo info = {
      /* sType = */ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      /* pNext = */ nullptr,
      /* flags = */ VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      // TODO(antiagainst): use a graphics + compute queue family.
      /* queueFamilyIndex = */ 0,
  };

  ::VkCommandPool raw_command_pool;
  LOG_ASSERT(
      ==, device.GetLogger(),
      device.vkCreateCommandPool(device, &info, nullptr, &raw_command_pool),
      VK_SUCCESS);
  return vulkan::VkCommandPool(raw_command_pool, nullptr, &device);
}
}
