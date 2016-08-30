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

#include "support/entry/entry.h"
#include "support/log/log.h"
#include "vulkan_helpers/helper_functions.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"

static_assert(VK_VERSION_1_0 == 1 && VK_HEADER_VERSION == 24,
              "review the following to make sure that all VkFormat values are "
              "covered after updating vulkan.h");

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");
  vulkan::LibraryWrapper wrapper(data->root_allocator, data->log.get());
  vulkan::VkInstance instance(vulkan::CreateEmptyInstance(&wrapper));
  containers::vector<VkPhysicalDevice> physical_devices(
      vulkan::GetPhysicalDevices(data->root_allocator, instance));
  const uint32_t physical_device_count = physical_devices.size();

  data->log->LogInfo("API: vkGetPhysicalDeviceFormatProperties");

  auto output_properties = [data](const VkFormatProperties& properties) {
    data->log->LogInfo("      linearTilingFeatures: ",
                       properties.linearTilingFeatures);
    data->log->LogInfo("      optimalTilingFeatures: ",
                       properties.optimalTilingFeatures);
    data->log->LogInfo("      bufferFeatures: ", properties.bufferFeatures);
  };

  {
    for (const auto& device : physical_devices) {
      data->log->LogInfo("  Phyiscal Device: ", device);

      VkFormatProperties properties;
      //                       0: VK_FORMAT_UNDEFINED
      //          1 -        184: assigned
      // 1000054000 - 1000054007: assigned
      for (uint64_t i = VK_FORMAT_BEGIN_RANGE; i <= VK_FORMAT_END_RANGE; ++i) {
        data->log->LogInfo("    VkFormat: ", i);
        instance.vkGetPhysicalDeviceFormatProperties(
            device, static_cast<VkFormat>(i), &properties);
        output_properties(properties);
      }
      for (uint64_t i = 1000054000ul; i <= 1000054007ul; ++i) {
        data->log->LogInfo("    VkFormat: ", i);
        instance.vkGetPhysicalDeviceFormatProperties(
            device, static_cast<VkFormat>(i), &properties);
        output_properties(properties);
      }
    }
  }

  data->log->LogInfo("Application Shutdown");
  return 0;
}
