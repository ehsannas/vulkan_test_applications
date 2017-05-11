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

uint32_t fragment_shader[] =
#include "simple_fragment.frag.spv"
    ;

uint32_t vertex_shader[] =
#include "simple_vertex.vert.spv"
    ;

namespace {
// Geometry data of the triangle to be drawn
const float kVertices[] = {
    -0.5, -0.5, 0.0,  // point 1
    0.5,  -0.5, 0.0,  // point 2
    0.0,  0.5,  0.0,  // point 3
};
const vulkan::VulkanGraphicsPipeline::InputStream kVerticesStream{
    0,                           // binding
    VK_FORMAT_R32G32B32_SFLOAT,  // format
    0,                           // offset
};

const float kUV[] = {
    0.0, 0.0,  // point 1
    1.0, 0.0,  // point 2
    0.0, 1.0,  // point 3
};
const vulkan::VulkanGraphicsPipeline::InputStream kUVStream{
    1,                        // binding
    VK_FORMAT_R32G32_SFLOAT,  // format
    0,                        // offset
};

const uint32_t kIndex[] = {0, 1, 2};

const VkCommandBufferBeginInfo kCommandBufferBeginInfo{
    /* sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    /* pNext = */ nullptr,
    /* flags = */ VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    /* pInheritanceInfo = */ nullptr,
};

// A helper function that returns a default buffer create info struct with
// given size and usage
VkBufferCreateInfo GetBufferCreateInfo(VkDeviceSize size,
                                       VkBufferUsageFlags usage) {
  return VkBufferCreateInfo{
      /* sType = */ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
      /* pNext = */ nullptr,
      /* flags = */ 0,
      /* size = */ size,
      /* usage = */ usage,
      /* sharingMode = */ VK_SHARING_MODE_EXCLUSIVE,
      /* queueFamilyIndexCount = */ 0,
      /* pQueueFamilyIndices = */ nullptr,
  };
}

// A helper function that flushes the given data to the device memory for the
// given host visible buffer. If a command buffer is given, add a buffer memory
// barrier to the command buffer.
void FlushHostVisibleBuffer(vulkan::VulkanApplication::Buffer* buf, size_t size,
                            const char* data, vulkan::VkCommandBuffer* cmd_buf,
                            VkPipelineStageFlags dst_stage_mask,
                            VkAccessFlags dst_access_flags) {
  char* p = reinterpret_cast<char*>(buf->base_address());
  for (size_t i = 0; i < buf->size() && i < size; i++) {
    p[i] = data[i];
  }
  buf->flush();

  if (cmd_buf) {
    VkBufferMemoryBarrier buf_barrier{
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        nullptr,
        VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT,
        dst_access_flags,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        *buf,
        0,
        VK_WHOLE_SIZE};

    (*cmd_buf)->vkCmdPipelineBarrier(
        *cmd_buf, VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT,
        dst_stage_mask, 0, 0, nullptr, 1, &buf_barrier, 0, nullptr);
  }
}

// A helper function that submit the given command buffer to the given queue
// and wait until the queue is in idle state.
void SubmitCommandBufferAndWaitIdle(const entry::entry_data* data,
                                    vulkan::VkQueue* queue_ptr,
                                    ::VkCommandBuffer cmd_buf) {
  auto& queue = *queue_ptr;
  VkSubmitInfo submit_info{
      VK_STRUCTURE_TYPE_SUBMIT_INFO,  // sType
      nullptr,                        // pNext
      0,                              // waitSemaphoreCount
      nullptr,                        // pWaitSemaphores
      nullptr,                        // pWaitDstStageMask,
      1,                              // commandBufferCount
      &cmd_buf,
      0,       // signalSemaphoreCount
      nullptr  // pSignalSemaphores
  };

  LOG_ASSERT(==, data->log,
             queue->vkQueueSubmit(queue, 1, &submit_info,
                                  static_cast<VkFence>(VK_NULL_HANDLE)),
             VK_SUCCESS);
  LOG_ASSERT(==, data->log, queue->vkQueueWaitIdle(queue), VK_SUCCESS);
}
}  // anonymous namespace

int main_entry(const entry::entry_data* data) {
  data->log->LogInfo("Application Startup");

  vulkan::VulkanApplication app(data->root_allocator, data->log.get(), data);
  vulkan::VkDevice& device = app.device();

  // Create vertex buffers and index buffer
  const auto vertices_buf_create_info = GetBufferCreateInfo(
      sizeof(kVertices),
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  const auto uv_buf_create_info =
      GetBufferCreateInfo(sizeof(kUV), VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  auto vertices_buf = app.CreateAndBindHostBuffer(&vertices_buf_create_info);
  auto uv_buf = app.CreateAndBindHostBuffer(&uv_buf_create_info);
  const auto index_buf_create_info =
      GetBufferCreateInfo(sizeof(kIndex), VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                              VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
  auto index_buf = app.CreateAndBindHostBuffer(&index_buf_create_info);
  const ::VkBuffer vertex_buffers[2] = {*vertices_buf, *uv_buf};
  const ::VkDeviceSize vertex_buffer_offsets[2] = {0, 0};

  // Create render pass.
  VkAttachmentReference color_attachment = {
      0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
  vulkan::VkRenderPass render_pass = app.CreateRenderPass(
      {{
          0,                                         // flags
          app.swapchain().format(),                  // format
          VK_SAMPLE_COUNT_1_BIT,                     // samples
          VK_ATTACHMENT_LOAD_OP_DONT_CARE,           // loadOp
          VK_ATTACHMENT_STORE_OP_STORE,              // storeOp
          VK_ATTACHMENT_LOAD_OP_DONT_CARE,           // stenilLoadOp
          VK_ATTACHMENT_STORE_OP_DONT_CARE,          // stenilStoreOp
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,  // initialLayout
          VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL   // finalLayout
      }},                                            // AttachmentDescriptions
      {{
          0,                                // flags
          VK_PIPELINE_BIND_POINT_GRAPHICS,  // pipelineBindPoint
          0,                                // inputAttachmentCount
          nullptr,                          // pInputAttachments
          1,                                // colorAttachmentCount
          &color_attachment,                // colorAttachment
          nullptr,                          // pResolveAttachments
          nullptr,                          // pDepthStencilAttachment
          0,                                // preserveAttachmentCount
          nullptr                           // pPreserveAttachments
      }},                                   // SubpassDescriptions
      {}                                    // SubpassDependencies
      );

  // Create pipeline
  vulkan::PipelineLayout pipeline_layout(app.CreatePipelineLayout({{}}));
  vulkan::VulkanGraphicsPipeline pipeline =
      app.CreateGraphicsPipeline(&pipeline_layout, &render_pass, 0);
  pipeline.AddShader(VK_SHADER_STAGE_VERTEX_BIT, "main", vertex_shader);
  pipeline.AddShader(VK_SHADER_STAGE_FRAGMENT_BIT, "main", fragment_shader);
  pipeline.SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
  pipeline.AddInputStream(sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX,
                          {kVerticesStream});
  pipeline.AddInputStream(sizeof(float), VK_VERTEX_INPUT_RATE_VERTEX,
                          {kUVStream});
  pipeline.SetScissor({{
                           0,  // offset.x
                           0,  // offset.y
                       },
                       {
                           app.swapchain().width(),  // extent.width
                           app.swapchain().height()  // extent.height
                       }});
  pipeline.SetViewport({
      0.0f,                                          // x
      0.0f,                                          // y
      static_cast<float>(app.swapchain().width()),   // width
      static_cast<float>(app.swapchain().height()),  // height
      0.0f,                                          // minDepth
      1.0f,                                          // maxDepth
  });
  pipeline.SetSamples(VK_SAMPLE_COUNT_1_BIT);
  pipeline.AddAttachment();
  pipeline.Commit();

  // Create image view
  VkImageViewCreateInfo image_view_create_info{
      VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,  // sType
      nullptr,                                   // pNext
      0,                                         // flags
      app.swapchain_images().front(),            // image
      VK_IMAGE_VIEW_TYPE_2D,                     // viewType
      app.swapchain().format(),                  // format
      {
          VK_COMPONENT_SWIZZLE_IDENTITY,  // components.r
          VK_COMPONENT_SWIZZLE_IDENTITY,  // components.g
          VK_COMPONENT_SWIZZLE_IDENTITY,  // components.b
          VK_COMPONENT_SWIZZLE_IDENTITY,  // components.a
      },
      {
          VK_IMAGE_ASPECT_COLOR_BIT,  // subresourceRange.aspectMask
          0,                          // subresourceRange.baseMipLevel
          1,                          // subresourceRange.levelCount
          0,                          // subresourceRange.baseArrayLayer
          1,                          // subresourceRange.layerCount
      },
  };
  ::VkImageView raw_image_view;
  LOG_ASSERT(==, data->log, app.device()->vkCreateImageView(
                                app.device(), &image_view_create_info, nullptr,
                                &raw_image_view),
             VK_SUCCESS);
  vulkan::VkImageView image_view(raw_image_view, nullptr, &app.device());

  // Create framebuffer
  VkFramebufferCreateInfo framebuffer_create_info{
      VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // sType
      nullptr,                                    // pNext
      0,                                          // flags
      render_pass,                                // renderPass
      1,                                          // attachmentCount
      &raw_image_view,                            // attachments
      app.swapchain().width(),                    // width
      app.swapchain().height(),                   // height
      1                                           // layers
  };
  ::VkFramebuffer raw_framebuffer;
  app.device()->vkCreateFramebuffer(app.device(), &framebuffer_create_info,
                                    nullptr, &raw_framebuffer);
  vulkan::VkFramebuffer framebuffer(raw_framebuffer, nullptr, &app.device());

  // Create render pass begin info
  VkClearValue clear_value{0};
  VkRenderPassBeginInfo pass_begin{
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,  // sType
      nullptr,                                   // pNext
      render_pass,                               // renderPass
      framebuffer,                               // framebuffer
      {{0, 0},
       {app.swapchain().width(), app.swapchain().height()}},  // renderArea
      1,                                                      // clearValueCount
      &clear_value                                            // clears
  };

  {
    // 1. vkCmdDraw
    auto cmd_buf = app.GetCommandBuffer();
    cmd_buf->vkBeginCommandBuffer(cmd_buf, &kCommandBufferBeginInfo);
    FlushHostVisibleBuffer(&*vertices_buf, sizeof(kVertices),
                           reinterpret_cast<const char*>(kVertices), &cmd_buf,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    FlushHostVisibleBuffer(&*uv_buf, sizeof(kUV),
                           reinterpret_cast<const char*>(kUV), &cmd_buf,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);

    cmd_buf->vkCmdBeginRenderPass(cmd_buf, &pass_begin,
                                  VK_SUBPASS_CONTENTS_INLINE);
    cmd_buf->vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                               pipeline);
    cmd_buf->vkCmdBindVertexBuffers(cmd_buf, 0, 2, vertex_buffers,
                                    vertex_buffer_offsets);
    cmd_buf->vkCmdDraw(cmd_buf, 3, 1, 0, 0);
    cmd_buf->vkCmdEndRenderPass(cmd_buf);
    cmd_buf->vkEndCommandBuffer(cmd_buf);
    SubmitCommandBufferAndWaitIdle(data, &app.render_queue(), cmd_buf);
  }

  {
    // 2. vkCmdDrawIndexed
    auto cmd_buf = app.GetCommandBuffer();
    cmd_buf->vkBeginCommandBuffer(cmd_buf, &kCommandBufferBeginInfo);
    FlushHostVisibleBuffer(&*vertices_buf, sizeof(kVertices),
                           reinterpret_cast<const char*>(kVertices), &cmd_buf,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    FlushHostVisibleBuffer(&*uv_buf, sizeof(kUV),
                           reinterpret_cast<const char*>(kUV), &cmd_buf,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    FlushHostVisibleBuffer(
        &*index_buf, sizeof(kIndex), reinterpret_cast<const char*>(kIndex),
        &cmd_buf, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_INDEX_READ_BIT);

    cmd_buf->vkCmdBeginRenderPass(cmd_buf, &pass_begin,
                                  VK_SUBPASS_CONTENTS_INLINE);
    cmd_buf->vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                               pipeline);
    cmd_buf->vkCmdBindVertexBuffers(cmd_buf, 0, 2, vertex_buffers,
                                    vertex_buffer_offsets);
    cmd_buf->vkCmdBindIndexBuffer(cmd_buf, *index_buf, 0, VK_INDEX_TYPE_UINT32);
    cmd_buf->vkCmdDrawIndexed(cmd_buf, 3, 1, 0, 0, 0);
    cmd_buf->vkCmdEndRenderPass(cmd_buf);
    cmd_buf->vkEndCommandBuffer(cmd_buf);
    SubmitCommandBufferAndWaitIdle(data, &app.render_queue(), cmd_buf);
  }

  {
    // 3. vkCmdDrawIndirect
    // Prepare indirect command
    const VkDrawIndirectCommand ic{3, 1, 0, 0};
    auto ic_buf_create_info = GetBufferCreateInfo(
        sizeof(ic),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    auto ic_buf = app.CreateAndBindHostBuffer(&ic_buf_create_info);

    auto cmd_buf = app.GetCommandBuffer();
    cmd_buf->vkBeginCommandBuffer(cmd_buf, &kCommandBufferBeginInfo);
    FlushHostVisibleBuffer(&*vertices_buf, sizeof(kVertices),
                           reinterpret_cast<const char*>(kVertices), &cmd_buf,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    FlushHostVisibleBuffer(&*uv_buf, sizeof(kUV),
                           reinterpret_cast<const char*>(kUV), &cmd_buf,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    FlushHostVisibleBuffer(&*ic_buf, sizeof(ic),
                           reinterpret_cast<const char*>(&ic), &cmd_buf,
                           VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                           VK_ACCESS_INDIRECT_COMMAND_READ_BIT);

    cmd_buf->vkCmdBeginRenderPass(cmd_buf, &pass_begin,
                                  VK_SUBPASS_CONTENTS_INLINE);
    cmd_buf->vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                               pipeline);
    cmd_buf->vkCmdBindVertexBuffers(cmd_buf, 0, 2, vertex_buffers,
                                    vertex_buffer_offsets);
    cmd_buf->vkCmdDrawIndirect(cmd_buf, *ic_buf, 0, 1, 0);
    cmd_buf->vkCmdEndRenderPass(cmd_buf);
    cmd_buf->vkEndCommandBuffer(cmd_buf);
    SubmitCommandBufferAndWaitIdle(data, &app.render_queue(), cmd_buf);
  }

  {
    // 4, vkCmdDrawIndexedIndirect
    const VkDrawIndexedIndirectCommand ic{3, 1, 0, 0, 0};
    auto ic_buf_create_info = GetBufferCreateInfo(
        sizeof(ic),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
    auto ic_buf = app.CreateAndBindHostBuffer(&ic_buf_create_info);

    auto cmd_buf = app.GetCommandBuffer();
    cmd_buf->vkBeginCommandBuffer(cmd_buf, &kCommandBufferBeginInfo);
    FlushHostVisibleBuffer(&*vertices_buf, sizeof(kVertices),
                           reinterpret_cast<const char*>(kVertices), &cmd_buf,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    FlushHostVisibleBuffer(&*uv_buf, sizeof(kUV),
                           reinterpret_cast<const char*>(kUV), &cmd_buf,
                           VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,
                           VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
    FlushHostVisibleBuffer(
        &*index_buf, sizeof(kIndex), reinterpret_cast<const char*>(kIndex),
        &cmd_buf, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_ACCESS_INDEX_READ_BIT);
    FlushHostVisibleBuffer(&*ic_buf, sizeof(ic),
                           reinterpret_cast<const char*>(&ic), &cmd_buf,
                           VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT,
                           VK_ACCESS_INDIRECT_COMMAND_READ_BIT);

    cmd_buf->vkCmdBeginRenderPass(cmd_buf, &pass_begin,
                                  VK_SUBPASS_CONTENTS_INLINE);
    cmd_buf->vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                               pipeline);
    cmd_buf->vkCmdBindVertexBuffers(cmd_buf, 0, 2, vertex_buffers,
                                    vertex_buffer_offsets);
    cmd_buf->vkCmdBindIndexBuffer(cmd_buf, *index_buf, 0, VK_INDEX_TYPE_UINT32);
    cmd_buf->vkCmdDrawIndexedIndirect(cmd_buf, *ic_buf, 0, 1, 0);
    cmd_buf->vkCmdEndRenderPass(cmd_buf);
    cmd_buf->vkEndCommandBuffer(cmd_buf);
    SubmitCommandBufferAndWaitIdle(data, &app.render_queue(), cmd_buf);
  }

  data->log->LogInfo("Application Shutdown");
  return 0;
}
