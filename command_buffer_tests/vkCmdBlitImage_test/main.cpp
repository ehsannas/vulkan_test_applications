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
#include "vulkan_helpers/vulkan_application.h"

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");

  vulkan::VulkanApplication application(data->root_allocator, data->log.get(),
                                        data, 1024 * 100, 1024 * 100,
                                        1024 * 100);

  VkExtent3D src_image_extent{32, 32, 1};
  VkImageCreateInfo src_image_create_info{
      /* sType = */ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      /* pNext = */ nullptr,
      /* flags = */ 0,
      /* imageType = */ VK_IMAGE_TYPE_2D,
      /* format = */ VK_FORMAT_R8G8B8A8_UNORM,
      /* extent = */ src_image_extent,
      /* mipLevels = */ 1,
      /* arrayLayers = */ 1,
      /* samples = */ VK_SAMPLE_COUNT_1_BIT,
      /* tiling = */ VK_IMAGE_TILING_OPTIMAL,
      /* usage = */ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
          VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
      /* sharingMode = */ VK_SHARING_MODE_EXCLUSIVE,
      /* queueFamilyIndexCount = */ 0,
      /* pQueueFamilyIndices = */ nullptr,
      /* initialLayout = */ VK_IMAGE_LAYOUT_UNDEFINED,
  };

  vulkan::ImagePointer src_image =
      application.CreateAndBindImage(&src_image_create_info);
  containers::vector<char> image_data(
      vulkan::GetImageExtentSizeInBytes(src_image_extent,
                                        VK_FORMAT_R8G8B8A8_UNORM),
      0xab, data->root_allocator);

  // Create semaphores, one for image data filling, another for layout
  // transitioning.
  VkSemaphoreCreateInfo image_fill_semaphore_create_info{
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
  ::VkSemaphore image_fill_semaphore;
  application.device()->vkCreateSemaphore(application.device(),
                                          &image_fill_semaphore_create_info,
                                          nullptr, &image_fill_semaphore);
  vulkan::VkSemaphore image_fill_semaphore_wrapper(
      image_fill_semaphore, nullptr, &application.device());
  VkSemaphoreCreateInfo layout_transition_semaphore_create_info{
      VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
  ::VkSemaphore layout_transition_semaphore;
  application.device()->vkCreateSemaphore(
      application.device(), &layout_transition_semaphore_create_info, nullptr,
      &layout_transition_semaphore);
  vulkan::VkSemaphore layout_transition_semaphore_wrapper(
      layout_transition_semaphore, nullptr, &application.device());

  vulkan::VkCommandBuffer fill_image_data_cmd_buf =
      application.GetCommandBuffer();
  bool fill_result = application.FillImageLayersData(
      src_image.get(),
      {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},   // subresourcelayer
      {0, 0, 0},                              // offset
      src_image_extent,                       // extent
      VK_IMAGE_LAYOUT_UNDEFINED,              // initial layout
      image_data,                             // data
      &fill_image_data_cmd_buf,               // command_buffer
      {},                                     // wait_semaphores
      {image_fill_semaphore},                 // signal_semaphores
      static_cast<::VkFence>(VK_NULL_HANDLE)  // fence
      );

  VkCommandBufferInheritanceInfo cmd_buf_hinfo{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
      nullptr,
      VK_NULL_HANDLE,
      0,
      VK_NULL_HANDLE,
      VK_FALSE,
      0,
      0};
  VkCommandBufferBeginInfo cmd_buf_begin_info{
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, 0, &cmd_buf_hinfo};
  vulkan::VkCommandBuffer layout_transition_cmd_buf =
      application.GetCommandBuffer();
  {
    // Image layout transition.
    vulkan::SetImageLayout(
        *src_image.get(), {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1},
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT,
        &layout_transition_cmd_buf, &application.render_queue(),
        {image_fill_semaphore}, {layout_transition_semaphore},
        static_cast<::VkFence>(VK_NULL_HANDLE), data->root_allocator);
  }

  {
    // 1. Blit to an image with exactly the same image create info as the source
    // image. And the source image has only one layer and one mip level.
    vulkan::ImagePointer dst_image =
        application.CreateAndBindImage(&src_image_create_info);
    VkImageBlit region{
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        {{0, 0, 0},
         {int32_t(src_image_extent.width), int32_t(src_image_extent.height),
          1}},
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
        {{0, 0, 0},
         {int32_t(src_image_extent.width), int32_t(src_image_extent.height),
          1}},
    };
    auto cmd_buf = application.GetCommandBuffer();
    cmd_buf->vkBeginCommandBuffer(cmd_buf, &cmd_buf_begin_info);
    cmd_buf->vkCmdBlitImage(
        cmd_buf, *src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *dst_image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region, VK_FILTER_LINEAR);
    cmd_buf->vkEndCommandBuffer(cmd_buf);

    VkCommandBuffer raw_cmd_buf = cmd_buf.get_command_buffer();
    const VkPipelineStageFlags wait_dst_stage_masks[1] = {
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT};
    VkSubmitInfo submit{VK_STRUCTURE_TYPE_SUBMIT_INFO,
                        nullptr,
                        1,
                        &layout_transition_semaphore,
                        wait_dst_stage_masks,
                        1,
                        &raw_cmd_buf,
                        0,
                        nullptr};

    application.render_queue()->vkQueueSubmit(
        application.render_queue(), 1, &submit,
        static_cast<VkFence>(VK_NULL_HANDLE));
    application.render_queue()->vkQueueWaitIdle(application.render_queue());
  }

  data->log->LogInfo("Application Shutdown");
  return 0;
}
