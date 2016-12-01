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
#include "vulkan_helpers/known_device_infos.h"
#include "vulkan_helpers/vulkan_application.h"
#include "vulkan_wrapper/queue_wrapper.h"

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");

  vulkan::VulkanApplication app(data->root_allocator, data->log.get(), data);
  // So we don't have to type app.device every time.
  vulkan::VkDevice& device = app.device();
  vulkan::VkQueue& render_queue = app.render_queue();

  {
    // 1. Wait on a single queue which will signal a fence.
    VkFenceCreateInfo create_info{
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,  // sType
        nullptr,                              // pNext
        0                                     // flags
    };
    VkFence fence;
    LOG_ASSERT(==, data->log.get(), VK_SUCCESS,
               device->vkCreateFence(device, &create_info, nullptr, &fence));
    render_queue->vkQueueSubmit(render_queue, 0, nullptr, fence);

    LOG_ASSERT(==, data->log.get(), VK_SUCCESS,
               device->vkDeviceWaitIdle(device));

    // vkWaitForFences() should return VK_SUCCESS immediately.
    LOG_ASSERT(==, data->log.get(), VK_SUCCESS,
               device->vkWaitForFences(device, 1, &fence, VK_FALSE, 0));
    device->vkDestroyFence(device, fence, nullptr);
  }

  {  // Destroy empty fence

    IF_NOT_DEVICE(data->log, device, vulkan::PixelC, 0x5A400000) {
      device->vkDestroyFence(device, (VkFence)VK_NULL_HANDLE, nullptr);
    }
  }

  data->log->LogInfo("Application Shutdown");
  return 0;
}
