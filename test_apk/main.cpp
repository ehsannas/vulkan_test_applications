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

#include <vector>

#include "support/entry/entry.h"
#include "support/log/log.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"

// Trival entry function.
// It will be removed in the future once we have more function
// apps to test.
// It makes sure that both the logging, and the dynamic loader are functioning.
int main_entry(const entry::entry_data *data) {
  data->log->LogInfo("Application Startup");
  vulkan::LibraryWrapper wrapper(data->log.get());

  uint32_t num_properties = 0;
  wrapper.vkEnumerateInstanceLayerProperties(&num_properties, nullptr);
  std::vector<VkLayerProperties> properties(num_properties);
  wrapper.vkEnumerateInstanceLayerProperties(&num_properties,
                                             properties.data());

  VkApplicationInfo app_info{
      VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, nullptr, 0, nullptr, 0, 0};

  VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                            nullptr,
                            0,
                            &app_info,
                            0,
                            nullptr,
                            0,
                            nullptr};

  VkInstance raw_instance;
  wrapper.vkCreateInstance(&info, nullptr, &raw_instance);

  {
    vulkan::VkInstance instance(raw_instance, nullptr, &wrapper);
  }
  data->log->LogInfo("Application Shutdown");
  return 0;
}