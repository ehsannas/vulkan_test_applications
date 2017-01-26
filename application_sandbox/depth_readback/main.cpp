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

#include "support/entry/entry.h"
#include "vulkan_helpers/helper_functions.h"
#include "vulkan_helpers/uniform_buffer.h"
#include "vulkan_helpers/vulkan_application.h"
#include "vulkan_helpers/vulkan_model.h"

#include <chrono>
#include "mathfu/matrix.h"
#include "mathfu/vector.h"

using Mat44 = mathfu::Matrix<float, 4, 4>;
using Vector4 = mathfu::Vector<float, 4>;

namespace cube_model {
#include "cube.obj.h"
}
const auto& cube_data = cube_model::model;

namespace plane_model {
#include "fullscreen_quad.obj.h"
}
const auto& plane_data = plane_model::model;

uint32_t cube_vertex_shader[] =
#include "cube.vert.spv"
    ;

uint32_t cube_fragment_shader[] =
#include "cube.frag.spv"
    ;

uint32_t plane_vertex_shader[] =
#include "plane.vert.spv"
    ;

uint32_t plane_fragment_shader[] =
#include "plane.frag.spv"
    ;

const static VkSampleCountFlagBits kVkSampleCount = VK_SAMPLE_COUNT_4_BIT;
const static VkFormat kDepthFormat = VK_FORMAT_D16_UNORM;
const static VkFormat kMultisampledFormat = VK_FORMAT_R8G8B8A8_UNORM;

// This creates an application wiht 16MB of image memory, and defaults
// for host, and device buffer sizes.
class DepthSample {
 public:
  DepthSample(const entry::entry_data* data)
      : app_(data->root_allocator, data->log.get(), data, {}, 1024 * 128,
             1024 * 1024 * 512, 1024 * 128),
        frame_data_(data->root_allocator),
        cube_(data->root_allocator, data->log.get(), cube_data.num_vertices,
              cube_data.positions, cube_data.uv, cube_data.normals,
              cube_data.num_indices, cube_data.indices),
        plane_(data->root_allocator, data->log.get(), plane_data.num_vertices,
               plane_data.positions, plane_data.uv, plane_data.normals,
               plane_data.num_indices, plane_data.indices),
        readyFence_(vulkan::CreateFence(&app_.device())) {
    const containers::vector<::VkImage>& swapchain_images =
        app_.swapchain_images();

    // TODO(awoloszyn): Fix this
    LOG_ASSERT(==, data->log.get(), false, app_.HasSeparatePresentQueue());

    frame_data_.reserve(swapchain_images.size());

    vulkan::VkCommandBuffer initialization_command_buffer =
        app_.GetCommandBuffer();

    VkCommandBufferBeginInfo begin_info = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,  // sType
        nullptr,                                      // pNext
        0,                                            // flags
        nullptr                                       // pInheritanceInfo
    };
    initialization_command_buffer->vkBeginCommandBuffer(
        initialization_command_buffer, &begin_info);

    vulkan::VkFence initialization_fence = vulkan::CreateFence(&app_.device());
    cube_.InitializeData(&app_, &initialization_command_buffer);
    plane_.InitializeData(&app_, &initialization_command_buffer);

    // TODO: Load the rest of the resources here
    // At least 2 more shaders
    // 1 renderpass
    //   2 subpasses (draw, depth->ms2)

    VkSubmitInfo init_submit_info{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
        nullptr,                        // pNext
        0,                              // waitSemaphoreCount
        nullptr,                        // pWaitSemaphores
        nullptr,                        // pWaitDstStageMask,
        1,                              // commandBufferCount
        &(initialization_command_buffer.get_command_buffer()),
        0,       // signalSemaphoreCount
        nullptr  // pSignalSemaphores
    };
    initialization_command_buffer->vkEndCommandBuffer(
        initialization_command_buffer);
    app_.render_queue()->vkQueueSubmit(app_.render_queue(), 1,
                                       &init_submit_info, initialization_fence);

    VkDescriptorSetLayoutBinding cube_descritor_set_layouts[2] = {
        {
            0,                                  // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // descriptorType
            1,                                  // descriptorCount
            VK_SHADER_STAGE_VERTEX_BIT,         // stageFlags
            nullptr                             // pImmutableSamplers
        },
        {
            1,                                  // binding
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,  // descriptorType
            1,                                  // descriptorCount
            VK_SHADER_STAGE_VERTEX_BIT,         // stageFlags
            nullptr                             // pImmutableSamplers
        }};

    pipeline_layout_ = containers::make_unique<vulkan::PipelineLayout>(
        data->root_allocator,
        app_.CreatePipelineLayout(
            {{cube_descritor_set_layouts[0], cube_descritor_set_layouts[1]}}));

    VkAttachmentReference color_attachment = {
        1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_attachment = {
        0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depth_read_attachment = {
        0, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL};

    VkViewport viewport{0.0f,
                        0.0f,
                        static_cast<float>(app_.swapchain().width()),
                        static_cast<float>(app_.swapchain().height()),
                        0.0f,
                        1.0f};

    VkRect2D scissor{{0, 0},
                     {app_.swapchain().width(), app_.swapchain().height()}};

    render_pass_ = containers::make_unique<vulkan::VkRenderPass>(
        data->root_allocator,
        app_.CreateRenderPass(
            {{
                 0,                                 // flags
                 kDepthFormat,                      // format
                 kVkSampleCount,                    // samples
                 VK_ATTACHMENT_LOAD_OP_CLEAR,       // loadOp
                 VK_ATTACHMENT_STORE_OP_STORE,      // storeOp
                 VK_ATTACHMENT_LOAD_OP_DONT_CARE,   // stenilLoadOp
                 VK_ATTACHMENT_STORE_OP_DONT_CARE,  // stenilStoreOp
                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,  // initialLayout
                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL  // finalLayout
             },
             {
                 0,                                         // flags
                 kMultisampledFormat,                       // format
                 kVkSampleCount,                            // samples
                 VK_ATTACHMENT_LOAD_OP_CLEAR,               // loadOp
                 VK_ATTACHMENT_STORE_OP_STORE,              // storeOp
                 VK_ATTACHMENT_LOAD_OP_DONT_CARE,           // stenilLoadOp
                 VK_ATTACHMENT_STORE_OP_DONT_CARE,          // stenilStoreOp
                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // initialLayout
                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL   // finalLayout
             }},  // AttachmentDescriptions
            {{
                0,                                // flags
                VK_PIPELINE_BIND_POINT_GRAPHICS,  // pipelineBindPoint
                0,                                // inputAttachmentCount
                nullptr,                          // pInputAttachments
                1,                                // colorAttachmentCount
                &color_attachment,                // colorAttachment
                nullptr,                          // pResolveAttachments
                &depth_attachment,                // pDepthStencilAttachment
                0,                                // preserveAttachmentCount
                nullptr                           // pPreserveAttachments
            }},                                   // SubpassDescriptions
            {}                                    // SubpassDependencies
            ));

    depth_read_render_pass_ = containers::make_unique<vulkan::VkRenderPass>(
        data->root_allocator,
        app_.CreateRenderPass(
            {{
                 0,                                 // flags
                 kDepthFormat,                      // format
                 kVkSampleCount,                    // samples
                 VK_ATTACHMENT_LOAD_OP_LOAD,        // loadOp
                 VK_ATTACHMENT_STORE_OP_STORE,      // storeOp
                 VK_ATTACHMENT_LOAD_OP_DONT_CARE,   // stenilLoadOp
                 VK_ATTACHMENT_STORE_OP_DONT_CARE,  // stenilStoreOp
                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,  // initialLayout
                 VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL  // finalLayout
             },
             {
                 0,                                         // flags
                 kMultisampledFormat,                       // format
                 kVkSampleCount,                            // samples
                 VK_ATTACHMENT_LOAD_OP_DONT_CARE,           // loadOp
                 VK_ATTACHMENT_STORE_OP_STORE,              // storeOp
                 VK_ATTACHMENT_LOAD_OP_DONT_CARE,           // stenilLoadOp
                 VK_ATTACHMENT_STORE_OP_DONT_CARE,          // stenilStoreOp
                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // initialLayout
                 VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL   // finalLayout
             }},  // AttachmentDescriptions
            {{
                0,                                // flags
                VK_PIPELINE_BIND_POINT_GRAPHICS,  // pipelineBindPoint
                1,                                // inputAttachmentCount
                &depth_read_attachment,           // pInputAttachments
                1,                                // colorAttachmentCount
                &color_attachment,                // colorAttachment
                nullptr,                          // pResolveAttachments
                nullptr,                          // pDepthStencilAttachment
                0,                                // preserveAttachmentCount
                nullptr                           // pPreserveAttachments
            }},                                   // SubpassDescriptions
            {}                                    // SubpassDependencies
            ));

    cube_pipeline_ = containers::make_unique<vulkan::VulkanGraphicsPipeline>(
        data->root_allocator,
        app_.CreateGraphicsPipeline(pipeline_layout_.get(), render_pass_.get(),
                                    0));
    cube_pipeline_->AddShader(VK_SHADER_STAGE_VERTEX_BIT, "main",
                              cube_vertex_shader);
    cube_pipeline_->AddShader(VK_SHADER_STAGE_FRAGMENT_BIT, "main",
                              cube_fragment_shader);
    cube_pipeline_->SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    cube_pipeline_->SetInputStreams(&cube_);
    cube_pipeline_->SetViewport(viewport);
    cube_pipeline_->SetScissor(scissor);
    cube_pipeline_->SetSamples(kVkSampleCount);
    cube_pipeline_->AddAttachment();
    cube_pipeline_->Commit();

    VkDescriptorSetLayoutBinding plane_descriptor_set_layout = {
        0,                                    // binding
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,  // descriptorType
        1,                                    // descriptorCount
        VK_SHADER_STAGE_FRAGMENT_BIT,         // descriptorCount
        nullptr                               // pImmutableSamplers
    };

    depth_pipeline_layout_ = containers::make_unique<vulkan::PipelineLayout>(
        data->root_allocator,
        app_.CreatePipelineLayout({{plane_descriptor_set_layout}}));

    depth_read_pipeline_ =
        containers::make_unique<vulkan::VulkanGraphicsPipeline>(
            data->root_allocator,
            app_.CreateGraphicsPipeline(depth_pipeline_layout_.get(),
                                        depth_read_render_pass_.get(), 0));
    depth_read_pipeline_->AddShader(VK_SHADER_STAGE_VERTEX_BIT, "main",
                                    plane_vertex_shader);
    depth_read_pipeline_->AddShader(VK_SHADER_STAGE_FRAGMENT_BIT, "main",
                                    plane_fragment_shader);
    depth_read_pipeline_->SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    depth_read_pipeline_->SetViewport(viewport);
    depth_read_pipeline_->SetScissor(scissor);
    depth_read_pipeline_->SetInputStreams(&plane_);
    depth_read_pipeline_->SetSamples(kVkSampleCount);
    depth_read_pipeline_->AddAttachment();
    depth_read_pipeline_->Commit();

    camera_data = containers::make_unique<vulkan::UniformData<camera_data_>>(
        data->root_allocator, &app_, swapchain_images.size());

    model_data = containers::make_unique<vulkan::UniformData<model_data_>>(
        data->root_allocator, &app_, swapchain_images.size());

    float aspect =
        (float)app_.swapchain().width() / (float)app_.swapchain().height();
    camera_data->data().projection_matrix =
        Mat44::FromScaleVector(mathfu::Vector<float, 3>{1.0f, -1.0f, 1.0f}) *
        Mat44::Perspective(1.5708, aspect, 0.1f, 100.0f);

    model_data->data().transform =
        Mat44::FromTranslationVector(mathfu::Vector<float, 3>{2.0, 2.0, -3.0});

    vulkan::VkCommandBuffer image_initialization_command_buffer =
        app_.GetCommandBuffer();

    image_initialization_command_buffer->vkBeginCommandBuffer(
        image_initialization_command_buffer, &begin_info);

    vulkan::VkFence image_initialization_fence =
        vulkan::CreateFence(&app_.device());

    for (size_t i = 0; i < swapchain_images.size(); ++i) {
      frame_data_.push_back((FrameData){
          swapchain_images[i], nullptr, nullptr, vulkan::ImagePointer(nullptr),
          vulkan::ImagePointer(nullptr), app_.GetCommandBuffer(), nullptr});

      frame_data_[i].cube_descriptor_set_ =
          containers::make_unique<vulkan::DescriptorSet>(
              data->root_allocator,
              app_.AllocateDescriptorSet({cube_descritor_set_layouts[0],
                                          cube_descritor_set_layouts[1]}));

      VkDescriptorBufferInfo buffer_infos[2] = {
          {
              camera_data->get_buffer(),             // buffer
              camera_data->get_offset_for_frame(i),  // offset
              camera_data->size(),                   // range
          },
          {
              model_data->get_buffer(),             // buffer
              model_data->get_offset_for_frame(i),  // offset
              model_data->size(),                   // range
          }};

      VkWriteDescriptorSet write{
          VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,  // sType
          nullptr,                                 // pNext
          *frame_data_[i].cube_descriptor_set_,    // dstSet
          0,                                       // dstbinding
          0,                                       // dstArrayElement
          2,                                       // descriptorCount
          VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,       // descriptorType
          nullptr,                                 // pImageInfo
          buffer_infos,                            // pBufferInfo
          nullptr,                                 // pTexelBufferView
      };

      app_.device()->vkUpdateDescriptorSets(app_.device(), 1, &write, 0,
                                            nullptr);

      VkImageCreateInfo image_create_info{
          /* sType = */ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
          /* pNext = */ nullptr,
          /* flags = */ 0,
          /* imageType = */ VK_IMAGE_TYPE_2D,
          /* format = */ kDepthFormat,
          /* extent = */ {
              /* width = */ app_.swapchain().width(),
              /* height = */ app_.swapchain().height(),
              /* depth = */ app_.swapchain().depth(),
          },
          /* mipLevels = */ 1,
          /* arrayLayers = */ 1,
          /* samples = */ kVkSampleCount,
          /* tiling = */ VK_IMAGE_TILING_OPTIMAL,
          /* usage = */ VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
              VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
          /* sharingMode = */ VK_SHARING_MODE_EXCLUSIVE,
          /* queueFamilyIndexCount = */ 0,
          /* pQueueFamilyIndices = */ nullptr,
          /* initialLayout = */ VK_IMAGE_LAYOUT_UNDEFINED,
      };

      frame_data_[i].depth_stencil_ =
          app_.CreateAndBindImage(&image_create_info);

      image_create_info.format = kMultisampledFormat;
      image_create_info.usage =
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

      frame_data_[i].multisampled_target_ =
          app_.CreateAndBindImage(&image_create_info);

      VkImageMemoryBarrier barriers[2] = {
          {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,            // sType
           nullptr,                                           // pNext
           0,                                                 // srcAccessMask
           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,      // dstAccessMask
           VK_IMAGE_LAYOUT_UNDEFINED,                         // oldLayout
           VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,  // newLayout
           VK_QUEUE_FAMILY_IGNORED,         // srcQueueFamilyIndex
           VK_QUEUE_FAMILY_IGNORED,         // dstQueueFamilyIndex
           *frame_data_[i].depth_stencil_,  // image
           {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}},
          {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    // sType
           nullptr,                                   // pNext
           0,                                         // srcAccessMask
           VK_ACCESS_MEMORY_READ_BIT,                 // dstAccessMask
           VK_IMAGE_LAYOUT_UNDEFINED,                 // oldLayout
           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // newLayout
           VK_QUEUE_FAMILY_IGNORED,                   // srcQueueFamilyIndex
           VK_QUEUE_FAMILY_IGNORED,                   // dstQueueFamilyIndex
           *frame_data_[i].multisampled_target_,      // image
           {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}}};

      image_initialization_command_buffer->vkCmdPipelineBarrier(
          image_initialization_command_buffer,
          VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
          0, 0, nullptr, 0, nullptr, 2, barriers);

      VkImageViewCreateInfo multisampled_view_create_info = {
          VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // sType
          nullptr,                                   // pNext
          0,                                         // flags
          *frame_data_[i].multisampled_target_,      // image
          VK_IMAGE_VIEW_TYPE_2D,                     // viewType
          kMultisampledFormat,                       // format
          {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
           VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

      ::VkImageView raw_views[3];
      LOG_ASSERT(==, data->log.get(), VK_SUCCESS,
                 app_.device()->vkCreateImageView(
                     app_.device(), &multisampled_view_create_info, nullptr,
                     &raw_views[1]));
      frame_data_[i].multisampled_view_ =
          containers::make_unique<vulkan::VkImageView>(
              data->root_allocator,
              vulkan::VkImageView(raw_views[1], nullptr, &app_.device()));

      VkImageViewCreateInfo depth_view_create = {
          VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // sType
          nullptr,                                   // pNext
          0,                                         // flags
          *frame_data_[i].depth_stencil_,            // image
          VK_IMAGE_VIEW_TYPE_2D,                     // viewType
          kDepthFormat,                              // format
          {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
           VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A},
          {VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1}};

      LOG_ASSERT(
          ==, data->log.get(), VK_SUCCESS,
          app_.device()->vkCreateImageView(app_.device(), &depth_view_create,
                                           nullptr, &raw_views[0]));
      frame_data_[i].depth_view_ = containers::make_unique<vulkan::VkImageView>(
          data->root_allocator,
          vulkan::VkImageView(raw_views[0], nullptr, &app_.device()));

      frame_data_[i].plane_descriptor_set_ =
          containers::make_unique<vulkan::DescriptorSet>(
              data->root_allocator,
              app_.AllocateDescriptorSet({plane_descriptor_set_layout}));
      VkDescriptorImageInfo image_info = {
          VK_NULL_HANDLE,                                  // sampler
          *frame_data_[i].depth_view_,                     // imageView
          VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL  // imageLayout
      };
      write.dstSet = *frame_data_[i].plane_descriptor_set_;
      write.descriptorCount = 1;
      write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
      write.pBufferInfo = nullptr;
      write.pImageInfo = &image_info;
      app_.device()->vkUpdateDescriptorSets(app_.device(), 1, &write, 0,
                                            nullptr);

      // Create a framebuffer with depth and image attachments
      VkFramebufferCreateInfo framebuffer_create_info{
          VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // sType
          nullptr,                                    // pNext
          0,                                          // flags
          *render_pass_,                              // renderPass
          2,                                          // attachmentCount
          raw_views,                                  // attachments
          app_.swapchain().width(),                   // width
          app_.swapchain().height(),                  // height
          1                                           // layers
      };

      ::VkFramebuffer raw_framebuffer;
      app_.device()->vkCreateFramebuffer(
          app_.device(), &framebuffer_create_info, nullptr, &raw_framebuffer);
      frame_data_[i].framebuffer_ =
          containers::make_unique<vulkan::VkFramebuffer>(
              data->root_allocator,
              vulkan::VkFramebuffer(raw_framebuffer, nullptr, &app_.device()));

      frame_data_[i].command_buffer_->vkBeginCommandBuffer(
          frame_data_[i].command_buffer_, &begin_info);
      vulkan::VkCommandBuffer& cmdBuffer = frame_data_[i].command_buffer_;

      VkClearValue clears[2];
      clears[0].depthStencil.depth = 1.0f;
      vulkan::ZeroMemory(&clears[1]);
      clears[1].color.float32[0] = 1.0f;

      VkRenderPassBeginInfo pass_begin = {
          VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,  // sType
          nullptr,                                   // pNext
          *render_pass_,                             // renderPass
          *frame_data_[i].framebuffer_,              // framebuffer
          {{0, 0},
           {app_.swapchain().width(),
            app_.swapchain().height()}},  // renderArea
          2,                              // clearValueCount
          clears                          // clears
      };

      VkImageMemoryBarrier barrier = {
          VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    // sType
          nullptr,                                   // pNext
          VK_ACCESS_MEMORY_READ_BIT,                 // srcAccessMask
          VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,      // dstAccessMask
          VK_IMAGE_LAYOUT_UNDEFINED,                 // oldLayout
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // newLayout
          VK_QUEUE_FAMILY_IGNORED,                   // srcQueueFamilyIndex
          VK_QUEUE_FAMILY_IGNORED,                   // dstQueueFamilyIndex
          *frame_data_[i].multisampled_target_,      // image
          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};

      cmdBuffer->vkCmdPipelineBarrier(
          cmdBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0,
          nullptr, 1, &barrier);

      cmdBuffer->vkCmdBeginRenderPass(cmdBuffer, &pass_begin,
                                      VK_SUBPASS_CONTENTS_INLINE);

      cmdBuffer->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   *cube_pipeline_);
      cmdBuffer->vkCmdBindDescriptorSets(
          cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
          ::VkPipelineLayout(*pipeline_layout_), 0, 1,
          &frame_data_[i].cube_descriptor_set_->raw_set(), 0, nullptr);
      cube_.Draw(&cmdBuffer);
      cmdBuffer->vkCmdEndRenderPass(cmdBuffer);

      pass_begin.renderPass = *depth_read_render_pass_;
      cmdBuffer->vkCmdBeginRenderPass(cmdBuffer, &pass_begin,
                                      VK_SUBPASS_CONTENTS_INLINE);
      cmdBuffer->vkCmdBindDescriptorSets(
          cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
          ::VkPipelineLayout(*depth_pipeline_layout_), 0, 1,
          &frame_data_[i].plane_descriptor_set_->raw_set(), 0, nullptr);
      cmdBuffer->vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                   *depth_read_pipeline_);
      plane_.Draw(&cmdBuffer);
      cmdBuffer->vkCmdEndRenderPass(cmdBuffer);

      VkImageMemoryBarrier resolve_barrier[2] = {
          {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,    // sType
           nullptr,                                   // pNext
           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,      // srcAccessMask
           VK_ACCESS_MEMORY_READ_BIT,                 // dstAccessMask
           VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // oldLayout
           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,      // newLayout
           VK_QUEUE_FAMILY_IGNORED,                   // srcQueueFamilyIndex
           VK_QUEUE_FAMILY_IGNORED,                   // dstQueueFamilyIndex
           *frame_data_[i].multisampled_target_,      // image
           {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}},
          {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
           nullptr,                                 // pNext
           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,    // srcAccessMask
           VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
           VK_IMAGE_LAYOUT_UNDEFINED,               // oldLayout
           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    // newLayout
           VK_QUEUE_FAMILY_IGNORED,                 // srcQueueFamilyIndex
           VK_QUEUE_FAMILY_IGNORED,                 // dstQueueFamilyIndex
           frame_data_[i].swapchain_image_,         // image
           {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}}};
      cmdBuffer->vkCmdPipelineBarrier(cmdBuffer,
                                      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                      VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
                                      nullptr, 0, nullptr, 2, resolve_barrier);

      if (kVkSampleCount != VK_SAMPLE_COUNT_1_BIT) {
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
                app_.swapchain().width(),   // width
                app_.swapchain().height(),  // height
                1                           // depth
            }                               // extent
        };
        cmdBuffer->vkCmdResolveImage(
            cmdBuffer, *frame_data_[i].multisampled_target_,
            VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            frame_data_[i].swapchain_image_,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
      } else {
        VkImageBlit blit_region = {
            {
                VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
                0,                          // mipLevel
                0,                          // baseArrayLayer
                1,                          // layerCount
            },                              // srcSubresource
            {{
                 0,  // x
                 0,  // y
                 0   // z
             },
             {
                 static_cast<int32_t>(app_.swapchain().width()),   // width
                 static_cast<int32_t>(app_.swapchain().height()),  // height
                 0                                                 // z
             }},                                                   // srcOffset
            {
                VK_IMAGE_ASPECT_COLOR_BIT,  // aspectMask
                0,                          // mipLevel
                0,                          // baseArrayLayer
                1,                          // layerCount
            },                              // dstSubresource
            {{
                 0,  // x
                 0,  // y
                 0   // z
             },
             {
                 static_cast<int32_t>(app_.swapchain().width()),   // width
                 static_cast<int32_t>(app_.swapchain().height()),  // height
                 0                                                 // z
             }}};
        cmdBuffer->vkCmdBlitImage(cmdBuffer,
                                  *frame_data_[i].multisampled_target_,
                                  VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                  frame_data_[i].swapchain_image_,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                                  &blit_region, VK_FILTER_NEAREST);
      }
      VkImageMemoryBarrier present_barrier = {
          VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,  // sType
          nullptr,                                 // pNext
          VK_ACCESS_TRANSFER_WRITE_BIT,            // srcAccessMask
          VK_ACCESS_MEMORY_READ_BIT,               // dstAccessMask
          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,    // oldLayout
          VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,         // newLayout
          VK_QUEUE_FAMILY_IGNORED,                 // srcQueueFamilyIndex
          VK_QUEUE_FAMILY_IGNORED,                 // dstQueueFamilyIndex
          frame_data_[i].swapchain_image_,         // image
          {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}};
      cmdBuffer->vkCmdPipelineBarrier(
          cmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
          VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1,
          &present_barrier);

      frame_data_[i].command_buffer_->vkEndCommandBuffer(
          frame_data_[i].command_buffer_);
    }

    image_initialization_command_buffer->vkEndCommandBuffer(
        image_initialization_command_buffer);

    VkSubmitInfo image_init_submit_info{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
        nullptr,                        // pNext
        0,                              // waitSemaphoreCount
        nullptr,                        // pWaitSemaphores
        nullptr,                        // pWaitDstStageMask,
        1,                              // commandBufferCount
        &(image_initialization_command_buffer.get_command_buffer()),
        0,       // signalSemaphoreCount
        nullptr  // pSignalSemaphores
    };

    app_.render_queue()->vkQueueSubmit(app_.render_queue(), 1,
                                       &image_init_submit_info,
                                       image_initialization_fence);

    ::VkFence all_fences[2] = {initialization_fence.get_raw_object(),
                               image_initialization_fence.get_raw_object()};

    app_.device()->vkWaitForFences(app_.device(), 2, all_fences, VK_TRUE,
                                   0xFFFFFFFFFFFFFFFF);
  }

  void Render(float time_since_last_render) {
    model_data->data().transform =
        model_data->data().transform *
        Mat44::FromRotationMatrix(
            Mat44::RotationX(3.14 * time_since_last_render) *
            Mat44::RotationY(3.14 * time_since_last_render * 0.5));

    app_.GetLogger()->LogInfo("Rendering frame <", time_since_last_render, ">");
    uint32_t image_idx;

    LOG_ASSERT(==, app_.GetLogger(), VK_SUCCESS,
               app_.device()->vkAcquireNextImageKHR(
                   app_.device(), app_.swapchain(), 0xFFFFFFFFFFFFFFFF,
                   static_cast<::VkSemaphore>(VK_NULL_HANDLE), readyFence_,
                   &image_idx));
    // TODO(awoloszyn): Swap out this logic for semaphores, we don't want to
    // stall the cpu to wait for the drawing to be done
    LOG_ASSERT(==, app_.GetLogger(), VK_SUCCESS,
               app_.device()->vkWaitForFences(app_.device(), 1,
                                              &readyFence_.get_raw_object(),
                                              VK_FALSE, 0xFFFFFFFFFFFFFFFF));
    LOG_ASSERT(==, app_.GetLogger(), VK_SUCCESS,
               app_.device()->vkResetFences(app_.device(), 1,
                                            &readyFence_.get_raw_object()));

    // Update our uniform buffers.
    camera_data->UpdateBuffer(&app_.render_queue(), image_idx);
    model_data->UpdateBuffer(&app_.render_queue(), image_idx);

    VkSubmitInfo init_submit_info{
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
        nullptr,                        // pNext
        0,                              // waitSemaphoreCount
        nullptr,                        // pWaitSemaphores
        nullptr,                        // pWaitDstStageMask,
        1,                              // commandBufferCount
        &(frame_data_[image_idx].command_buffer_.get_command_buffer()),
        0,       // signalSemaphoreCount
        nullptr  // pSignalSemaphores
    };

    // TODO(awoloszyn): Swap out this logic for semaphores, we don't want to
    // stall the cpu to wait for the drawing to be done
    app_.render_queue()->vkQueueSubmit(app_.render_queue(), 1,
                                       &init_submit_info,
                                       readyFence_.get_raw_object());

    app_.device()->vkWaitForFences(app_.device(), 1,
                                   &readyFence_.get_raw_object(), VK_FALSE,
                                   0xFFFFFFFFFFFFFFFF);
    LOG_ASSERT(==, app_.GetLogger(), VK_SUCCESS,
               app_.device()->vkResetFences(app_.device(), 1,
                                            &readyFence_.get_raw_object()));

    VkPresentInfoKHR present_info{
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,  // sType
        nullptr,                             // pNext
        0,                                   // waitSemaphoreCount
        nullptr,                             // pWaitSemaphores
        1,                                   // swapchainCount
        &app_.swapchain().get_raw_object(),  // pSwapchains
        &image_idx,                          // pImageIndices
        nullptr,                             // pResults
    };
    LOG_ASSERT(==, app_.GetLogger(),
               app_.present_queue()->vkQueuePresentKHR(app_.present_queue(),
                                                       &present_info),
               VK_SUCCESS);
  }

 private:
  struct FrameData {
    ::VkImage swapchain_image_;
    containers::unique_ptr<vulkan::VkImageView> multisampled_view_;
    containers::unique_ptr<vulkan::VkImageView> depth_view_;
    vulkan::ImagePointer depth_stencil_;
    vulkan::ImagePointer multisampled_target_;
    vulkan::VkCommandBuffer command_buffer_;
    containers::unique_ptr<vulkan::VkFramebuffer> framebuffer_;
    containers::unique_ptr<vulkan::VkFramebuffer> depth_write_framebufer_;
    containers::unique_ptr<vulkan::DescriptorSet> cube_descriptor_set_;
    containers::unique_ptr<vulkan::DescriptorSet> plane_descriptor_set_;
  };

  struct camera_data_ {
    bool operator==(const camera_data_& other) {
      for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
          if (projection_matrix(i, j) != other.projection_matrix(i, j)) {
            return false;
          }
        }
      }
      return true;
    }
    Mat44 projection_matrix;
  };

  struct model_data_ {
    bool operator==(const model_data_& other) {
      for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
          if (transform(i, j) != other.transform(i, j)) {
            return false;
          }
        }
      }
      return true;
    }
    Mat44 transform;
  };

  vulkan::VulkanApplication app_;
  containers::vector<FrameData> frame_data_;

  containers::unique_ptr<vulkan::PipelineLayout> pipeline_layout_;
  containers::unique_ptr<vulkan::PipelineLayout> depth_pipeline_layout_;
  containers::unique_ptr<vulkan::VulkanGraphicsPipeline> cube_pipeline_;
  containers::unique_ptr<vulkan::VulkanGraphicsPipeline> depth_read_pipeline_;
  containers::unique_ptr<vulkan::VkRenderPass> render_pass_;
  containers::unique_ptr<vulkan::VkRenderPass> depth_read_render_pass_;

  vulkan::VulkanModel cube_;
  vulkan::VulkanModel plane_;

  containers::unique_ptr<vulkan::UniformData<camera_data_>> camera_data;
  containers::unique_ptr<vulkan::UniformData<model_data_>> model_data;

  vulkan::VkFence readyFence_;  // TODO switch this out later, this is
                                //   not actually a good thing to do
};

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");
  DepthSample sample(data);

  auto last_frame_time = std::chrono::high_resolution_clock::now();

  while (true) {
    auto new_frame_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed_time =
        new_frame_time - last_frame_time;
    last_frame_time = new_frame_time;
    sample.Render(elapsed_time.count());
  }

  data->log->LogInfo("Application Shutdown");
}