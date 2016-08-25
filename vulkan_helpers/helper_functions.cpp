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

#include "support/log/log.h"

namespace vulkan {
VkInstance CreateEmptyInstance(LibraryWrapper *wrapper) {
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

std::vector<VkPhysicalDevice> GetPhysicalDevices(VkInstance &instance) {
  uint32_t device_count = 0;
  instance.vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  std::vector<VkPhysicalDevice> physical_devices(device_count);
  LOG_ASSERT(==, instance.GetLogger(),
             instance.vkEnumeratePhysicalDevices(instance, &device_count,
                                                 physical_devices.data()),
             VK_SUCCESS);

  return std::move(physical_devices);
}

VkDevice CreateDefaultDevice(VkInstance &instance) {
  std::vector<VkPhysicalDevice> physical_devices = GetPhysicalDevices(instance);
  float priority = 1.f;

  // TODO(awoloszyn): Return the first graphics + compute queue.
  VkDeviceQueueCreateInfo queue_info{
      VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, &priority};

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
      instance.vkCreateDevice(physical_devices[0], &info, nullptr, &raw_device),
      VK_SUCCESS);
  return vulkan::VkDevice(raw_device, nullptr, &instance);
}
}