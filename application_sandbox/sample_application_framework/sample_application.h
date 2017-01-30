// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef SAMPLE_APPLICATION_FRAMEWORK_SAMPLE_APPLICATION_H_
#define SAMPLE_APPLICATION_FRAMEWORK_SAMPLE_APPLICATION_H_

#include "support/entry/entry.h"
#include "vulkan_helpers/helper_functions.h"
#include "vulkan_helpers/vulkan_application.h"

#include <chrono>
#include <cstddef>

namespace sample_application {

const static VkSampleCountFlagBits kVkMultiSampledSampleCount =
    VK_SAMPLE_COUNT_4_BIT;
const static VkFormat kDepthFormat = VK_FORMAT_D16_UNORM;
const static VkFormat kMultisampledFormat = VK_FORMAT_R8G8B8A8_UNORM;

struct SampleOptions {
  bool enable_multisampling = false;
  bool enable_depth_buffer = false;

  SampleOptions& EnableMultisampling() {
    enable_multisampling = true;
    return *this;
  }
  SampleOptions& EnableDepthBuffer() {
    enable_depth_buffer = true;
    return *this;
  }
};

const VkCommandBufferBeginInfo kBeginCommandBuffer = {
    VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
    nullptr,                                      // pNext
    0,                                            // flags
    nullptr                                       // pInheritanceInfo
};

const VkSubmitInfo kEmptySubmitInfo{
    VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
    nullptr,                        // pNext
    0,                              // waitSemaphoreCount
    nullptr,                        // pWaitSemaphores
    nullptr,                        // pWaitDstStageMask,
    0,                              // commandBufferCount
    nullptr,
    0,       // signalSemaphoreCount
    nullptr  // pSignalSemaphores
};

template <typename FrameData>
class Sample {
  struct SampleFrameData {
    ::VkImage swapchain_image_;
    containers::unique_ptr<vulkan::VkImageView> image_view;
    containers::unique_ptr<vulkan::VkImageView> depth_view_;
    containers::unique_ptr<vulkan::VkCommandBuffer> setup_command_buffer_;
    containers::unique_ptr<vulkan::VkCommandBuffer> resolve_command_buffer_;
    vulkan::ImagePointer depth_stencil_;
    vulkan::ImagePointer multisampled_target_;
    FrameData child_data_;
  };

 public:
  Sample(containers::Allocator* allocator, const entry::entry_data* entry_data,
         uint32_t host_buffer_size_in_MB, uint32_t image_memory_size_in_MB,
         uint32_t device_buffer_size_in_MB, const SampleOptions& options)
      : options_(options),
        data_(entry_data),
        allocator_(allocator),
        application_(allocator, entry_data->log.get(), entry_data, {}, {0},
                     host_buffer_size_in_MB * 1024 * 1024,
                     image_memory_size_in_MB * 1024 * 1024,
                     device_buffer_size_in_MB * 1024 * 1024),
        frame_data_(allocator),
        swapchain_images_(application_.swapchain_images()),
        readyFence_(vulkan::CreateFence(&application_.device())),
        last_frame_time_(std::chrono::high_resolution_clock::now()) {
    // TODO(awoloszyn): Fix this
    LOG_ASSERT(==, app()->GetLogger(), false,
               application_.HasSeparatePresentQueue());

    frame_data_.reserve(swapchain_images_.size());
    render_target_format_ = options.enable_multisampling
                                ? kMultisampledFormat
                                : application_.swapchain().format();
    num_samples_ = options.enable_multisampling ? kVkMultiSampledSampleCount
                                                : VK_SAMPLE_COUNT_1_BIT;
    default_viewport_ = {0.0f,
                         0.0f,
                         static_cast<float>(application_.swapchain().width()),
                         static_cast<float>(application_.swapchain().height()),
                         0.0f,
                         1.0f};

    default_scissor_ = {
        {0, 0},
        {application_.swapchain().width(), application_.swapchain().height()}};
  }

  void Initialize() {
    vulkan::VkFence initialization_fence =
        vulkan::CreateFence(&application_.device());

    vulkan::VkCommandBuffer initialization_command_buffer =
        application_.GetCommandBuffer();
    initialization_command_buffer->vkBeginCommandBuffer(
        initialization_command_buffer, &kBeginCommandBuffer);

    InitializeApplicationData(&initialization_command_buffer,
                              swapchain_images_.size());

    for (size_t i = 0; i < swapchain_images_.size(); ++i) {
      frame_data_.push_back(SampleFrameData());
      InitializeLocalFrameData(&frame_data_.back(),
                               &initialization_command_buffer, i);
    }

    initialization_command_buffer->vkEndCommandBuffer(
        initialization_command_buffer);

    VkSubmitInfo submit_info = kEmptySubmitInfo;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers =
        &(initialization_command_buffer.get_command_buffer());
    application_.render_queue()->vkQueueSubmit(
        application_.render_queue(), 1, &submit_info, initialization_fence);

    application_.device()->vkWaitForFences(
        application_.device(), 1, &initialization_fence.get_raw_object(),
        VK_TRUE, 0xFFFFFFFFFFFFFFFF);
  }

  // The format that we are using to render. This will be either the swapchain
  // format if we are not rendering multi-sampled, or the multisampled image
  // format if we are rendering multi-sampled.
  VkFormat render_format() const { return render_target_format_; }

  VkFormat depth_format() const { return kDepthFormat; }

  // The number of samples that we are rendering with.
  VkSampleCountFlagBits num_samples() const { return num_samples_; }

  vulkan::VulkanApplication* app() { return &application_; }

  const VkViewport& viewport() const { return default_viewport_; }
  const VkRect2D& scissor() const { return default_scissor_; }

  void ProcessFrame() {
    auto current_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed_time = current_time - last_frame_time_;
    last_frame_time_ = current_time;
    Update(elapsed_time.count());

    uint32_t image_idx;

    LOG_ASSERT(==, app()->GetLogger(), VK_SUCCESS,
               app()->device()->vkAcquireNextImageKHR(
                   app()->device(), app()->swapchain(), 0xFFFFFFFFFFFFFFFF,
                   static_cast<::VkSemaphore>(VK_NULL_HANDLE), readyFence_,
                   &image_idx));
    // TODO(awoloszyn): Swap out this logic for semaphores, we don't want to
    // stall the cpu to wait for the drawing to be done
    LOG_ASSERT(==, app()->GetLogger(), VK_SUCCESS,
               app()->device()->vkWaitForFences(app()->device(), 1,
                                                &readyFence_.get_raw_object(),
                                                VK_FALSE, 0xFFFFFFFFFFFFFFFF));
    LOG_ASSERT(==, app()->GetLogger(), VK_SUCCESS,
               app()->device()->vkResetFences(app()->device(), 1,
                                              &readyFence_.get_raw_object()));
    app()->GetLogger()->LogInfo("Rendering frame <", elapsed_time.count(),
                                ">: <", image_idx, ">");

    VkSubmitInfo init_submit_info{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
        nullptr,                        // pNext
        0,                              // waitSemaphoreCount
        nullptr,                        // pWaitSemaphores
        nullptr,                        // pWaitDstStageMask,
        1,                              // commandBufferCount
        &(frame_data_[image_idx].setup_command_buffer_->get_command_buffer()),
        0,       // signalSemaphoreCount
        nullptr  // pSignalSemaphores
    };

    // TODO(awoloszyn): Swap out this logic for semaphores, we don't want to
    // stall the cpu to wait for the drawing to be done
    app()->render_queue()->vkQueueSubmit(
        app()->render_queue(), 1, &init_submit_info,
        static_cast<::VkFence>(VK_NULL_HANDLE));

    Render(&app()->render_queue(), image_idx,
           &frame_data_[image_idx].child_data_);
    init_submit_info.pCommandBuffers =
        &(frame_data_[image_idx].resolve_command_buffer_->get_command_buffer());

    app()->render_queue()->vkQueueSubmit(app()->render_queue(), 1,
                                         &init_submit_info,
                                         readyFence_.get_raw_object());

    app()->device()->vkWaitForFences(app()->device(), 1,
                                     &readyFence_.get_raw_object(), VK_FALSE,
                                     0xFFFFFFFFFFFFFFFF);
    LOG_ASSERT(==, app()->GetLogger(), VK_SUCCESS,
               app()->device()->vkResetFences(app()->device(), 1,
                                              &readyFence_.get_raw_object()));

    VkPresentInfoKHR present_info{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,    // sType
        nullptr,                               // pNext
        0,                                     // waitSemaphoreCount
        nullptr,                               // pWaitSemaphores
        1,                                     // swapchainCount
        &app()->swapchain().get_raw_object(),  // pSwapchains
        &image_idx,                            // pImageIndices
        nullptr,                               // pResults
    };
    LOG_ASSERT(==, app()->GetLogger(),
               app()->present_queue()->vkQueuePresentKHR(app()->present_queue(),
                                                         &present_info),
               VK_SUCCESS);
  }

 private:
  const size_t sample_frame_data_offset =
      reinterpret_cast<size_t>(
          &(reinterpret_cast<SampleFrameData*>(4096)->child_data_)) -
      4096;

 public:
  const ::VkImageView& depth_view(FrameData* data) {
    SampleFrameData* base = reinterpret_cast<SampleFrameData*>(
        reinterpret_cast<uint8_t*>(data) - sample_frame_data_offset);
    return base->depth_view_->get_raw_object();
  }

  const ::VkImageView& color_view(FrameData* data) {
    SampleFrameData* base = reinterpret_cast<SampleFrameData*>(
        reinterpret_cast<uint8_t*>(data) - sample_frame_data_offset);
    return base->image_view->get_raw_object();
  }

 private:
  virtual void InitializeFrameData(
      FrameData* data, vulkan::VkCommandBuffer* initialization_buffer,
      size_t frame_index) = 0;
  virtual void InitializeApplicationData(
      vulkan::VkCommandBuffer* initialization_buffer,
      size_t num_swapchain_images) = 0;

  virtual void Update(float time_since_last_render) = 0;
  virtual void Render(vulkan::VkQueue* queue, size_t frame_index,
                      FrameData* data) = 0;

  void InitializeLocalFrameData(SampleFrameData* data,
                                vulkan::VkCommandBuffer* initialization_buffer,
                                size_t frame_index) {
    data->swapchain_image_ = swapchain_images_[frame_index];

    VkImageCreateInfo image_create_info{
        /* sType = */
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        /* pNext = */ nullptr,
        /* flags = */ 0,
        /* imageType = */ VK_IMAGE_TYPE_2D,
        /* format = */ kDepthFormat,
        /* extent = */
        {
            /* width = */ application_.swapchain().width(),
            /* height = */ application_.swapchain().height(),
            /* depth = */ application_.swapchain().depth(),
        },
        /* mipLevels = */ 1,
        /* arrayLayers = */ 1,
        /* samples = */ num_samples_,
        /* tiling = */
        VK_IMAGE_TILING_OPTIMAL,
        /* usage = */
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        /* sharingMode = */
        VK_SHARING_MODE_EXCLUSIVE,
        /* queueFamilyIndexCount = */ 0,
        /* pQueueFamilyIndices = */ nullptr,
        /* initialLayout = */
        VK_IMAGE_LAYOUT_UNDEFINED,
    };

    VkImageViewCreateInfo view_create_info = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // sType
        nullptr,                                   // pNext
        0,                                         // flags
        VK_NULL_HANDLE,                            // image
        VK_IMAGE_VIEW_TYPE_2D,                     // viewType
        kDepthFormat,                              // format
        {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
         VK_COMPONENT_SWIZZLE_A},
        {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}};

    ::VkImageView raw_view;

    if (options_.enable_depth_buffer) {
      data->depth_stencil_ =
          application_.CreateAndBindImage(&image_create_info);
      view_create_info.image = *data->depth_stencil_;

      LOG_ASSERT(
          ==, data_->log.get(), VK_SUCCESS,
          application_.device()->vkCreateImageView(
              application_.device(), &view_create_info, nullptr, &raw_view));
      data->depth_view_ = containers::make_unique<vulkan::VkImageView>(
          allocator_,
          vulkan::VkImageView(raw_view, nullptr, &application_.device()));
    }

    if (options_.enable_multisampling) {
      image_create_info.format = render_target_format_;
      image_create_info.usage =
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

      data->multisampled_target_ =
          application_.CreateAndBindImage(&image_create_info);
    }

    view_create_info.image = options_.enable_multisampling
                                 ? *data->multisampled_target_
                                 : data->swapchain_image_;
    view_create_info.format = render_target_format_;
    view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    LOG_ASSERT(
        ==, data_->log.get(), VK_SUCCESS,
        application_.device()->vkCreateImageView(
            application_.device(), &view_create_info, nullptr, &raw_view));
    data->image_view = containers::make_unique<vulkan::VkImageView>(
        allocator_,
        vulkan::VkImageView(raw_view, nullptr, &application_.device()));

    VkImageMemoryBarrier barriers[2] = {
        {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,            // sType
         nullptr,                                           // pNext
         0,                                                 // srcAccessMask
         VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,      // dstAccessMask
         VK_IMAGE_LAYOUT_UNDEFINED,                         // oldLayout
         VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,  // newLayout
         VK_QUEUE_FAMILY_IGNORED,  // srcQueueFamilyIndex
         VK_QUEUE_FAMILY_IGNORED,  // dstQueueFamilyIndex
         options_.enable_depth_buffer
             ? static_cast<::VkImage>(*data->depth_stencil_)
             : static_cast<::VkImage>(VK_NULL_HANDLE),  // image
         {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}},
        {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    // sType
         nullptr,                                   // pNext
         0,                                         // srcAccessMask
         VK_ACCESS_MEMORY_READ_BIT,                 // dstAccessMask
         VK_IMAGE_LAYOUT_UNDEFINED,                 // oldLayout
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // newLayout
         VK_QUEUE_FAMILY_IGNORED,                   // srcQueueFamilyIndex
         VK_QUEUE_FAMILY_IGNORED,                   // dstQueueFamilyIndex
         options_.enable_multisampling ? *data->multisampled_target_
                                       : data->swapchain_image_,  // image
         {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}}};

    (*initialization_buffer)
        ->vkCmdPipelineBarrier(
            (*initialization_buffer), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT, 0, 0, nullptr, 0, nullptr,
            options_.enable_depth_buffer ? 2 : 1,
            &barriers[options_.enable_depth_buffer ? 0 : 1]);

    VkImageMemoryBarrier barrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    // sType
        nullptr,                                   // pNext
        VK_ACCESS_MEMORY_READ_BIT,                 // srcAccessMask
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,      // dstAccessMask
        VK_IMAGE_LAYOUT_UNDEFINED,                 // oldLayout
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // newLayout
        VK_QUEUE_FAMILY_IGNORED,                   // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                   // dstQueueFamilyIndex
        options_.enable_multisampling ? *data->multisampled_target_
                                      : data->swapchain_image_,  // image
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

    data->setup_command_buffer_ =
        containers::make_unique<vulkan::VkCommandBuffer>(
            allocator_, app()->GetCommandBuffer());

    (*data->setup_command_buffer_)
        ->vkBeginCommandBuffer((*data->setup_command_buffer_),
                               &kBeginCommandBuffer);

    (*data->setup_command_buffer_)
        ->vkCmdPipelineBarrier((*data->setup_command_buffer_),
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0,
                               0, nullptr, 0, nullptr, 1, &barrier);
    (*data->setup_command_buffer_)
        ->vkEndCommandBuffer(*data->setup_command_buffer_);

    data->resolve_command_buffer_ =
        containers::make_unique<vulkan::VkCommandBuffer>(
            allocator_, app()->GetCommandBuffer());

    (*data->resolve_command_buffer_)
        ->vkBeginCommandBuffer((*data->resolve_command_buffer_),
                               &kBeginCommandBuffer);
    VkImageLayout old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    if (options_.enable_multisampling) {
      old_layout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      VkImageMemoryBarrier resolve_barrier[2] = {
          {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    // sType
           nullptr,                                   // pNext
           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,      // srcAccessMask
           VK_ACCESS_MEMORY_READ_BIT,                 // dstAccessMask
           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // oldLayout
           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,      // newLayout
           VK_QUEUE_FAMILY_IGNORED,                   // srcQueueFamilyIndex
           VK_QUEUE_FAMILY_IGNORED,                   // dstQueueFamilyIndex
           *data->multisampled_target_,               // image
           {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}},
          {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
           nullptr,                                 // pNext
           VK_ACCESS_MEMORY_READ_BIT,               // srcAccessMask
           VK_ACCESS_TRANSFER_WRITE_BIT,            // dstAccessMask
           VK_IMAGE_LAYOUT_UNDEFINED,               // oldLayout
           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    // newLayout
           VK_QUEUE_FAMILY_IGNORED,                 // srcQueueFamilyIndex
           VK_QUEUE_FAMILY_IGNORED,                 // dstQueueFamilyIndex
           data->swapchain_image_,                  // image
           {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}}};
      (*data->resolve_command_buffer_)
          ->vkCmdPipelineBarrier(*data->resolve_command_buffer_,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr,
                                 0, nullptr, 2, resolve_barrier);
      VkImageResolve region = {
          {
              VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
              0,                          // mipLevel
              0,                          // baseArrayLayer
              1,                          // layerCount
          },                              // srcSubresource
          {
              0,  // x
              0,  // y
              0   // z
          },      // srcOffset
          {
              VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
              0,                          // mipLevel
              0,                          // baseArrayLayer
              1,                          // layerCount
          },                              // dstSubresource
          {
              0,  // x
              0,  // y
              0   // z
          },      // dstOffset
          {
              app()->swapchain().width(),   // width
              app()->swapchain().height(),  // height
              1                             // depth
          }                                 // extent
      };
      (*data->resolve_command_buffer_)
          ->vkCmdResolveImage(
              (*data->resolve_command_buffer_), *data->multisampled_target_,
              VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, data->swapchain_image_,
              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    }

    VkImageMemoryBarrier present_barrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
        nullptr,                                 // pNext
        VK_ACCESS_TRANSFER_WRITE_BIT,            // srcAccessMask
        VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
        old_layout,                              // oldLayout
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
        VK_QUEUE_FAMILY_IGNORED,                 // srcQueueFamilyIndex
        VK_QUEUE_FAMILY_IGNORED,                 // dstQueueFamilyIndex
        data->swapchain_image_,                  // image
        {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
    (*data->resolve_command_buffer_)
        ->vkCmdPipelineBarrier((*data->resolve_command_buffer_),
                               VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                               VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0,
                               nullptr, 0, nullptr, 1, &present_barrier);
    (*data->resolve_command_buffer_)
        ->vkEndCommandBuffer(*data->resolve_command_buffer_);

    InitializeFrameData(&data->child_data_, initialization_buffer, frame_index);
  }

  SampleOptions options_;
  const entry::entry_data* data_;
  containers::Allocator* allocator_;
  containers::vector<SampleFrameData> frame_data_;
  vulkan::VulkanApplication application_;
  VkSampleCountFlagBits num_samples_;
  VkFormat render_target_format_;
  VkViewport default_viewport_;
  VkRect2D default_scissor_;
  std::chrono::time_point<std::chrono::high_resolution_clock> last_frame_time_;

  // Do not move these above application_, they rely on the fact that
  // application_ will be initialized first.
  const containers::vector<::VkImage>& swapchain_images_;
  vulkan::VkFence readyFence_;  // TODO switch this out later, this is
                                //   not actually a good thing to do
};
}

#endif  // SAMPLE_APPLICATION_FRAMEWORK_SAMPLE_APPLICATION_H_