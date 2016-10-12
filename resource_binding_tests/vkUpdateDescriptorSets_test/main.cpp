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
#include "vulkan_helpers/known_device_infos.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"
#include "vulkan_wrapper/sub_objects.h"

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");

  auto& allocator = data->root_allocator;
  vulkan::LibraryWrapper wrapper(allocator, data->log.get());
  vulkan::VkInstance instance(vulkan::CreateEmptyInstance(allocator, &wrapper));
  vulkan::VkDevice device(vulkan::CreateDefaultDevice(allocator, instance));

  {  // 1. Zero writes and zero copies.
    device->vkUpdateDescriptorSets(device, 0, nullptr, 0, nullptr);
  }

  {  // 2. One write and zero copies.
    vulkan::VkDescriptorPool pool = vulkan::CreateDescriptorPool(
        &device, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, 1);
    ::VkDescriptorPool raw_pool = pool.get_raw_object();

    vulkan::VkDescriptorSetLayout layout = vulkan::CreateDescriptorSetLayout(
        &device, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
    ::VkDescriptorSetLayout raw_layout = layout.get_raw_object();

    vulkan::VkDescriptorSet set = vulkan::AllocateDescriptorSet(
        &device, raw_pool, raw_layout, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
    ::VkDescriptorSet raw_set = set.get_raw_object();

    const VkBufferCreateInfo info = {
        /* sType = */ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        /* pNext = */ nullptr,
        /* flags = */ 0,
        /* size = */ 1024,
        /* usage = */ VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        /* sharingMode = */ VK_SHARING_MODE_EXCLUSIVE,
        /* queueFamilyIndexCount = */ 0,
        /* pQueueFamilyIndices = */ nullptr,
    };

    ::VkBuffer raw_buffer;
    device->vkCreateBuffer(device, &info, nullptr, &raw_buffer);
    vulkan::VkBuffer(raw_buffer, nullptr, &device);

    const VkDescriptorBufferInfo bufinfo[2] = {
        {
            /* buffer = */ raw_buffer,
            /* offset = */ 0,
            /* range = */ 512,
        },
        {
            /* buffer = */ raw_buffer,
            /* offset = */ 512,
            /* range = */ 512,
        },
    };

    const VkWriteDescriptorSet write = {
        /* sType = */ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        /* pNext = */ nullptr,
        /* dstSet = */ raw_set,
        /* dstBinding = */ 0,
        /* dstArrayElement = */ 0,
        /* descriptorCount = */ 2,
        /* descriptorType = */ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        /* pImageInfo = */ nullptr,
        /* pBufferInfo = */ bufinfo,
        /* pTexelBufferView = */ nullptr,
    };

    device->vkUpdateDescriptorSets(device, 1, &write, 0, nullptr);
  }

  data->log->LogInfo("Application Shutdown");
  return 0;
}
