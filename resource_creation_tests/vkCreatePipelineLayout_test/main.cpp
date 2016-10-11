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
#include "vulkan_helpers/structs.h"
#include "vulkan_helpers/vulkan_application.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");

  vulkan::VulkanApplication app(data->root_allocator, data->log.get(), data);
  // So we don't have to type app.device every time.
  vulkan::VkDevice& device = app.device();

  {
    // Empty pipeline layout
    VkPipelineLayoutCreateInfo create_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // sType
        nullptr,                                        // pNext
        0,                                              // flags
        0,                                              // setLayoutCount
        nullptr,                                        // pSetLayouts
        0,       // pushConstantRangeCount
        nullptr  // pPushConstantRanges
    };
    VkPipelineLayout raw_pipeline_layout;
    LOG_ASSERT(==, data->log, VK_SUCCESS,
               device->vkCreatePipelineLayout(device, &create_info, nullptr,
                                              &raw_pipeline_layout));
    device->vkDestroyPipelineLayout(device, raw_pipeline_layout, nullptr);
  }

  {
    // Single layout
    vulkan::VkDescriptorSetLayout layout = vulkan::CreateDescriptorSetLayout(
        data->root_allocator, &device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         1, VK_SHADER_STAGE_ALL, nullptr}});
    VkDescriptorSetLayout raw_layouts[1] = {layout};
    VkPipelineLayoutCreateInfo create_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // sType
        nullptr,                                        // pNext
        0,                                              // flags
        1,                                              // setLayoutCount
        raw_layouts,                                    // pSetLayouts
        0,       // pushConstantRangeCount
        nullptr  // pPushConstantRanges
    };
    VkPipelineLayout raw_pipeline_layout;
    LOG_ASSERT(==, data->log, VK_SUCCESS,
               device->vkCreatePipelineLayout(device, &create_info, nullptr,
                                              &raw_pipeline_layout));
    device->vkDestroyPipelineLayout(device, raw_pipeline_layout, nullptr);
  }

  {
    // 2 layouts
    vulkan::VkDescriptorSetLayout layout = vulkan::CreateDescriptorSetLayout(
        data->root_allocator, &device, {{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         1, VK_SHADER_STAGE_ALL, nullptr}});
    vulkan::VkDescriptorSetLayout layout2 = vulkan::CreateDescriptorSetLayout(
        data->root_allocator, &device, {{0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                         1, VK_SHADER_STAGE_ALL, nullptr}});
    VkDescriptorSetLayout raw_layouts[2] = {layout, layout2};

    VkPipelineLayoutCreateInfo create_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // sType
        nullptr,                                        // pNext
        0,                                              // flags
        2,                                              // setLayoutCount
        raw_layouts,                                    // pSetLayouts
        0,       // pushConstantRangeCount
        nullptr  // pPushConstantRanges
    };
    VkPipelineLayout raw_pipeline_layout;
    LOG_ASSERT(==, data->log, VK_SUCCESS,
               device->vkCreatePipelineLayout(device, &create_info, nullptr,
                                              &raw_pipeline_layout));
    device->vkDestroyPipelineLayout(device, raw_pipeline_layout, nullptr);
  }

  {
    // Empty destroy
    IF_NOT_DEVICE(data->log, device, vulkan::PixelC, 0x5A400000) {
      device->vkDestroyPipelineLayout(
          device, static_cast<VkPipelineLayout>(VK_NULL_HANDLE), nullptr);
    }
  }

  data->log->LogInfo("Application Shutdown");
  return 0;
}
