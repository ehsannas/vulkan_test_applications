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
#include "vulkan_helpers/structs.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");
  vulkan::LibraryWrapper wrapper(data->root_allocator, data->log.get());
  vulkan::VkInstance instance(
      vulkan::CreateEmptyInstance(data->root_allocator, &wrapper));
  containers::vector<VkPhysicalDevice> physical_devices(
      vulkan::GetPhysicalDevices(data->root_allocator, instance));

  data->log->LogInfo("API: vkGetPhysicalDeviceImageFormatProperties");
  auto& allocator = data->root_allocator;

  auto IsExpectedReturnCode = [](const VkResult result) -> bool {
    return result == VK_SUCCESS || result == VK_ERROR_FORMAT_NOT_SUPPORTED ||
           result == VK_ERROR_OUT_OF_HOST_MEMORY ||
           result == VK_ERROR_OUT_OF_DEVICE_MEMORY;
  };

  for (const auto& device : physical_devices) {
    data->log->LogInfo("  Phyiscal Device: ", device);
    uint64_t counter = 0;

    VkImageFormatProperties properties;
    for (auto format : vulkan::AllVkFormats(allocator)) {
      for (auto type : vulkan::AllVkImageTypes(allocator)) {
        for (auto tiling : vulkan::AllVkImageTilings(allocator)) {
          for (auto usage :
               vulkan::AllVkImageUsageFlagCombinations(allocator)) {
            if (usage == 0) continue;
            for (auto flags :
                 vulkan::AllVkImageCreateFlagCombinations(allocator)) {
              const VkResult result =
                  instance->vkGetPhysicalDeviceImageFormatProperties(
                      device, format, type, tiling, usage, flags, &properties);
              LOG_EXPECT(==, data->log, IsExpectedReturnCode(result), true);
              // Periodically output to avoid flood.
              if ((counter++ & 0x3ffff) == 0) {
                data->log->LogInfo("    format: ", format);
                data->log->LogInfo("    type: ", type);
                data->log->LogInfo("    tiling: ", tiling);
                data->log->LogInfo("    usage: ", usage);
                data->log->LogInfo("    flags: ", flags);
                data->log->LogInfo("      maxMipLevels: ",
                                   properties.maxMipLevels);
                data->log->LogInfo("      maxArrayLayers: ",
                                   properties.maxArrayLayers);
              }

              if (result == VK_SUCCESS) {
                // TODO(antiagainst): flesh out tests for
                // vkGetPhysicalDeviceImageFormatProperties here
              }
            }
          }
        }
      }
    }
  }

  data->log->LogInfo("Application Shutdown");
  return 0;
}
