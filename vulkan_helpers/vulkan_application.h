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

#ifndef VULKAN_HELPERS_VULKAN_APPLICATION
#define VULKAN_HELPERS_VULKAN_APPLICATION

#include "support/containers/allocator.h"
#include "support/containers/ordered_multimap.h"
#include "support/containers/vector.h"
#include "support/entry/entry.h"
#include "support/log/log.h"
#include "vulkan_helpers/helper_functions.h"
#include "vulkan_wrapper/command_buffer_wrapper.h"
#include "vulkan_wrapper/device_wrapper.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"
#include "vulkan_wrapper/queue_wrapper.h"
#include "vulkan_wrapper/sub_objects.h"

namespace vulkan {
class VulkanModel;
struct AllocationToken;

// This class represents a location in GPU memory for storing data.
// You can suballocate memory from this region, and return memory to the
// arena for future use.
class VulkanArena {
 public:
  // If map==true then the memory for this Arena is mapped to a host-visible
  // address.
  VulkanArena(containers::Allocator* allocator, logging::Logger* log,
              ::VkDeviceSize buffer_size, uint32_t memory_type_index,
              VkDevice* device, bool map);
  ~VulkanArena();

  // Returns an AllocationToken for the memory of a given size and alignment.
  // Fills *memory, and *offset with the ::VkDeviceMemory and ::VkDeviceSize
  // representing the allocation location. If base_address is not nullptr,
  // sets *base_address to the host-visible address of the returned memory, or
  // nullptr if the memory was not mappable.
  AllocationToken* AllocateMemory(::VkDeviceSize size, ::VkDeviceSize alignment,
                                  ::VkDeviceMemory* memory,
                                  ::VkDeviceSize* offset, char** base_address);

  // Frees the memory pointed to by the AllocationToken.
  void FreeMemory(AllocationToken* token);

 private:
  containers::Allocator* allocator_;
  containers::ordered_multimap<::VkDeviceSize, AllocationToken*> freeblocks_;
  AllocationToken* first_block_;
  char* base_address_;
  ::VkDevice device_;
  LazyDeviceFunction<PFN_vkUnmapMemory>* unmap_memory_function_;
  VkDeviceMemory memory_;
  logging::Logger* log_;
};

class VulkanApplication;
class PipelineLayout;

// Customizable Graphics pipeline state.
// Defaults to the following properties:
//    Dynamic Viewport & Scissor
//    POLYGON_MODE_FILL
//    Backface culling
//    Clockwise winding
//    non-multisampled
//    Depth testing enabled
//    Depth write enabled
//    Rasterization enabled
//    Stencil test disabled
//    Opaque Color blending

class VulkanGraphicsPipeline {
 public:
  struct InputStream {
    uint32_t binding;
    VkFormat format;
    uint32_t offset;
  };

  VulkanGraphicsPipeline(containers::Allocator* allocator,
                         PipelineLayout* layout, VulkanApplication* application,
                         VkRenderPass* render_pass, uint32_t subpass);
  VulkanGraphicsPipeline(containers::Allocator* allocator)
      : stages_(allocator),
        dynamic_states_(allocator),
        vertex_binding_descriptions_(allocator),
        vertex_attribute_descriptions_(allocator),
        shader_modules_(allocator),
        attachments_(allocator),
        pipeline_(VK_NULL_HANDLE, nullptr, nullptr) {}

  VulkanGraphicsPipeline(VulkanGraphicsPipeline&& other) = default;

  template <int N>
  void AddShader(VkShaderStageFlagBits stage, const char* entry,
                 uint32_t (&code)[N]) {
    return AddShader(stage, entry, code, N);
  }

  void AddShader(VkShaderStageFlagBits stage, const char* entry, uint32_t* code,
                 uint32_t numCodeWords);

  // patch_size is unused unless there is a tessellation shader.
  void SetTopology(VkPrimitiveTopology topology, uint32_t patch_size = 0);
  void SetViewport(const VkViewport& viewport);
  void SetScissor(const VkRect2D& scissor);

  void SetSamples(VkSampleCountFlagBits samples);

  void SetCullMode(VkCullModeFlagBits mode);

  // Sets the rasterization fill mode of the polygon.
  void SetRasterizationFill(VkPolygonMode mode);

  // Adds an attachment to this pipeline. Sets up default opaque blending
  // for that attachment.
  void AddAttachment();

  // Adds an attachment to this pipeline.
  void AddAttachment(const VkPipelineColorBlendAttachmentState& state);

  // Adds a dynamic state to this pipeline.
  void AddDynamicState(VkDynamicState dynamic_state);

  // Adds a vertex input stream to this pipeline, with the given
  // bindings.
  void AddInputStream(uint32_t stride, VkVertexInputRate input_rate,
                      std::initializer_list<InputStream> inputs);

  // Sets the vertex input streams from the given model.
  void SetInputStreams(VulkanModel* model);

  void Commit();
  operator ::VkPipeline() const { return pipeline_; }

 private:
  ::VkRenderPass render_pass_;
  uint32_t subpass_;
  VulkanApplication* application_;
  containers::vector<VkPipelineShaderStageCreateInfo> stages_;
  VkPipelineVertexInputStateCreateInfo vertex_input_state_;
  VkPipelineInputAssemblyStateCreateInfo input_assembly_state_;
  VkPipelineTessellationStateCreateInfo tessellation_state_;
  VkPipelineViewportStateCreateInfo viewport_state_;
  VkPipelineRasterizationStateCreateInfo rasterization_state_;
  VkPipelineMultisampleStateCreateInfo multisample_state_;
  VkPipelineDepthStencilStateCreateInfo depth_stencil_state_;
  VkPipelineColorBlendStateCreateInfo color_blend_state_;
  VkPipelineDynamicStateCreateInfo dynamic_state_;
  VkViewport viewport_;
  VkRect2D scissor_;
  containers::vector<VkDynamicState> dynamic_states_;
  containers::vector<VkVertexInputBindingDescription>
      vertex_binding_descriptions_;
  containers::vector<VkVertexInputAttributeDescription>
      vertex_attribute_descriptions_;
  containers::vector<VkShaderModule> shader_modules_;
  containers::vector<VkPipelineColorBlendAttachmentState> attachments_;
  ::VkPipelineLayout layout_;
  VkPipeline pipeline_;
  uint32_t contained_stages_;
};

// PipelineLayout holds a VkPipelineLayout object as well as as set of
// VkDescriptorSetLayout objects used to create that pipeline layout.
class PipelineLayout {
 public:
  PipelineLayout(PipelineLayout&& other) = default;
  PipelineLayout(const PipelineLayout& other) = delete;

  operator VkPipelineLayout&() { return pipeline_layout_; }
  operator ::VkPipelineLayout() { return pipeline_layout_; }

 private:
  // TODO(awoloszyn): Handle push constants here too
  PipelineLayout(
      containers::Allocator* allocator, VkDevice* device,
      std::initializer_list<std::initializer_list<VkDescriptorSetLayoutBinding>>
          layouts)
      : pipeline_layout_(VK_NULL_HANDLE, nullptr, device),
        descriptor_set_layouts_(allocator) {
    containers::vector<::VkDescriptorSetLayout> raw_layouts(allocator);
    raw_layouts.reserve(layouts.size());

    descriptor_set_layouts_.reserve(layouts.size());
    for (auto binding_list : layouts) {
      descriptor_set_layouts_.emplace_back(
          CreateDescriptorSetLayout(allocator, device, binding_list));
      raw_layouts.push_back(descriptor_set_layouts_.back());
    }
    VkPipelineLayoutCreateInfo create_info = {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // sType
        nullptr,                                        // pNext
        0,                                              // flags
        static_cast<uint32_t>(raw_layouts.size()),      // setLayoutCount
        raw_layouts.data(),                             // pSetLayouts
        0,        // pushConstantRangeCount
        nullptr,  // pPushConstantRanges
    };

    ::VkPipelineLayout layout;
    LOG_ASSERT(==, device->GetLogger(), VK_SUCCESS,
               (*device)->vkCreatePipelineLayout(*device, &create_info, nullptr,
                                                 &layout));
    pipeline_layout_.initialize(layout);
  }
  friend class VulkanApplication;
  containers::vector<VkDescriptorSetLayout> descriptor_set_layouts_;
  VkPipelineLayout pipeline_layout_;
};

// DescriptorSet holds a VkDescriptorSet object and the pool and layout used for
// allocating it.
class DescriptorSet {
 public:
  operator ::VkDescriptorSet() const { return set_; }

  const ::VkDescriptorSet& raw_set() const { return set_.get_raw_object(); }
  ::VkDescriptorPool pool() const { return pool_.get_raw_object(); }
  ::VkDescriptorSetLayout layout() const { return layout_.get_raw_object(); }

 private:
  friend class VulkanApplication;

  static VkDescriptorPool CreateDescriptorPool(
      containers::Allocator* allocator, VkDevice* device,
      std::initializer_list<VkDescriptorSetLayoutBinding> bindings);

  // Creates a descriptor set with one descriptor according to the given
  // |binding|.
  DescriptorSet(containers::Allocator* allocator, VkDevice* device,
                std::initializer_list<VkDescriptorSetLayoutBinding> bindings);

  // Pools is designed to amortize the cost of descriptor set allocation.
  // But here we create a dedicated pool for each descriptor set. It suffers
  // for performance, but is easy to write.
  VkDescriptorPool pool_;
  VkDescriptorSetLayout layout_;
  VkDescriptorSet set_;
};

// VulkanApplication holds all of the data needed for a typical single-threaded
// Vulkan application.
class VulkanApplication {
 public:
  // The Image class holds onto a VkImage as well as memory that is bound to it.
  // When it is destroyed, it will return the memory to the heap from which
  // it was created.
  class Image {
   public:
    operator ::VkImage() const { return image_; }
    const ::VkImage& get_raw_image() const { return image_.get_raw_object(); }
    ~Image() { heap_->FreeMemory(token_); }
    VkFormat format() const { return format_; }

   private:
    friend class ::vulkan::VulkanApplication;
    Image(VulkanArena* heap, AllocationToken* token, VkImage&& image,
          VkFormat format)
        : heap_(heap),
          token_(token),
          image_(std::move(image)),
          format_(format) {}
    VulkanArena* heap_;
    AllocationToken* token_;
    VkImage image_;
    VkFormat format_;
  };

  // The buffer class holds onto a VkBuffer. If this buffer was created
  // in a host-visible heap, then the host-visible address can be
  // retreved using base_address().
  class Buffer {
   public:
    operator ::VkBuffer() const { return buffer_; }
    ~Buffer() { heap_->FreeMemory(token_); }

    // Returns the base_address of the host-visible section of memory.
    // Returns nullptr if the host-visible memory is not available.
    char* base_address() const { return base_address_; }

    // If this is host-visible memory, flushes the range so that
    // writes are visible to the GPU.
    void flush() {
      if (flush_memory_range_) {
        VkMappedMemoryRange range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                  nullptr, memory_, offset_, size_};
        (*flush_memory_range_)(device_, 1, &range);
      }
    }

    // If this is host-visible memory, flushes only the given range so that
    // writes are visible to the GPU
    void flush(size_t offset, size_t size) {
      if (flush_memory_range_) {
        VkMappedMemoryRange range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                  nullptr, memory_, offset_ + offset, size};
        (*flush_memory_range_)(device_, 1, &range);
      }
    }

    // if this is host-visible memory, invalidates the range so that
    // GPU writes become visible.
    void invalidate() {
      if (invalidate_memory_range_) {
        VkMappedMemoryRange range{VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
                                  nullptr, memory_, offset_, size_};
        (*invalidate_memory_range_)(device_, 1, &range);
      }
    }

   private:
    friend class ::vulkan::VulkanApplication;
    Buffer(
        VulkanArena* heap, AllocationToken* token, VkBuffer&& buffer,
        char* base_address, ::VkDevice device, ::VkDeviceMemory memory,
        ::VkDeviceSize offset, ::VkDeviceSize size,
        LazyDeviceFunction<PFN_vkFlushMappedMemoryRanges>* flush_memory_range,
        LazyDeviceFunction<PFN_vkInvalidateMappedMemoryRanges>*
            invalidate_memory_range)
        : base_address_(base_address),
          heap_(heap),
          token_(token),
          buffer_(std::move(buffer)),
          device_(device),
          memory_(memory),
          offset_(offset),
          size_(size),
          flush_memory_range_(flush_memory_range),
          invalidate_memory_range_(invalidate_memory_range) {}
    char* base_address_;
    VulkanArena* heap_;
    AllocationToken* token_;
    VkBuffer buffer_;
    ::VkDevice device_;
    ::VkDeviceMemory memory_;
    ::VkDeviceSize offset_;
    ::VkDeviceSize size_;
    LazyDeviceFunction<PFN_vkFlushMappedMemoryRanges>* flush_memory_range_;
    LazyDeviceFunction<PFN_vkInvalidateMappedMemoryRanges>*
        invalidate_memory_range_;
  };

  // On creation creates an instance, device, surface, swapchain, queues,
  // and command pool for the application.
  // It also creates 3 memory arenas with the given sizes.
  //  One for host-visible buffers.
  //  One for device-only-accessible buffers.
  //  One for device-only images.
  VulkanApplication(containers::Allocator* allocator, logging::Logger* log,
                    const entry::entry_data* entry_data,
                    const std::initializer_list<const char*> extensions = {},
                    const VkPhysicalDeviceFeatures& features = {0},
                    uint32_t host_buffer_size = 1024 * 128,
                    uint32_t device_image_size = 1024 * 128,
                    uint32_t device_buffer_size = 1024 * 128);

  // Creates an image from the given create_info, and binds memory from the
  // device-only image Arena.
  containers::unique_ptr<Image> CreateAndBindImage(
      const VkImageCreateInfo* create_info);
  // Creates a buffer from the given create_info, and binds memory from the
  // host-visible buffer Arena. Also maps the memory needed for the device.
  containers::unique_ptr<Buffer> CreateAndBindHostBuffer(
      const VkBufferCreateInfo* create_info);
  // Creates a buffer from the given create_info, and binds memory from the
  // device-only-accessible buffer Arena.
  containers::unique_ptr<Buffer> CreateAndBindDeviceBuffer(
      const VkBufferCreateInfo* create_info);

  // Creates a command buffer, appends commands to fill the given |data| to the
  // specified |image| and submit the command buffer to application's render
  // queue. If succeed, returns true and the command buffer. The operations
  // recorded in the command buffer will wait until |wait_semaphores| signals.
  // Once the operation is done, |signal_semaphores| and |fence| will be
  // signaled. The target image layout will be changed to
  // VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL. If the operation can not be done
  // successfully, this method returns false and a command buffer wrapping
  // VK_NULL_HANDLE, the layout of the image will not be changed.
  std::tuple<bool, VkCommandBuffer> FillImageLayersData(
      Image* img, const VkImageSubresourceLayers& image_subresource,
      const VkOffset3D& image_offset, const VkExtent3D& image_extent,
      VkImageLayout initial_img_layout, const containers::vector<uint8_t>& data,
      std::initializer_list<::VkSemaphore> wait_semaphores,
      std::initializer_list<::VkSemaphore> signal_semaphores, ::VkFence fence);

  // Fills a small buffer with the given data.
  // This inserts a series of calls to vkCmdUpdateBuffer into the given
  // command_buffer, so it is
  // expected that this be used for buffers that are usually < 65536 bytes.
  // buffer must have been created with the VK_BUFFER_USAGE_TRANSFER_DST_BIT.
  // Synchronization must be used to ensure that data remains valid until
  // the command buffer is done with it.
  void FillSmallBuffer(Buffer* buffer, const void* data, size_t data_size,
                       size_t buffer_offset, VkCommandBuffer* command_buffer,
                       VkAccessFlags target_usage);

  // Dump the data in the specific layers of the given image to the provided
  // vector. The operation will wait for the |wait_semaphores| to begin and
  // will wait until all the commands are executed then returns. This function
  // returns true and changes the source image layout to
  // VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL if the operation is done
  // successfully, otherwise returns false and keeps the layout unchanged.
  bool DumpImageLayersData(
      Image* img, const VkImageSubresourceLayers& image_subresource,
      const VkOffset3D& image_offset, const VkExtent3D& image_extent,
      VkImageLayout initial_img_layout, containers::vector<uint8_t>* data,
      std::initializer_list<::VkSemaphore> wait_semaphores);

  // Creates and returns a new primary level CommandBuffer using the
  // Application's default VkCommandPool.
  VkCommandBuffer GetCommandBuffer() {
    return CreateCommandBuffer(&command_pool_, VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                               &device_);
  }

  // Creates and returns a new CommandBuffer with given command buffer level
  // using the Application's default VkCommandPool
  VkCommandBuffer GetCommandBuffer(VkCommandBufferLevel level) {
    return CreateCommandBuffer(&command_pool_, level, &device_);
  }

  // Returns the Graphics and Compute queue for this application.
  VkQueue& render_queue() { return *render_queue_; }

  // Returns the Present queue for this application. Note: It may be the same
  // as the render queue.
  VkQueue& present_queue() { return *present_queue_; }

  // Returns the device that was created for this application.
  VkDevice& device() { return device_; }
  VkInstance& instance() { return instance_; }

  VkPipelineCache& pipeline_cache() { return pipeline_cache_; }

  logging::Logger* GetLogger() { return log_; }

  // Creates and returns a shader module from the given spirv code.
  template <int size>
  VkShaderModule CreateShaderModule(uint32_t (&vals)[size]) {
    VkShaderModuleCreateInfo create_info{
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,  // sType
        nullptr,                                      // pNext
        0,                                            // flags
        4 * size,                                     // codeSize
        vals                                          // pCode
    };
    ::VkShaderModule module;
    LOG_ASSERT(
        ==, log_, VK_SUCCESS,
        device_->vkCreateShaderModule(device_, &create_info, nullptr, &module));
    return VkShaderModule(module, nullptr, &device_);
  }

  // Returns true if the Present queue is not the same as the present queue.
  bool HasSeparatePresentQueue() const {
    return present_queue_ != render_queue_;
  }

  // Creates and returns a PipelineLayout from the given
  // DescriptorSetLayoutBindings
  PipelineLayout CreatePipelineLayout(
      std::initializer_list<std::initializer_list<VkDescriptorSetLayoutBinding>>
          layouts) {
    return PipelineLayout(allocator_, &device_, layouts);
  }

  // Allocates a descriptor set with one descriptor according to the given
  // |binding|.
  DescriptorSet AllocateDescriptorSet(
      std::initializer_list<VkDescriptorSetLayoutBinding> bindings) {
    return DescriptorSet(allocator_, &device_, bindings);
  }

  VkSwapchainKHR& swapchain() { return swapchain_; }

  containers::vector<::VkImage>& swapchain_images() {
    return swapchain_images_;
  }
  // Creates a render pass, from the given VkAttachmentDescriptions,
  // VkSubpassDescriptions, and VkSubpassDependencies
  VkRenderPass CreateRenderPass(
      std::initializer_list<VkAttachmentDescription> attachments,
      std::initializer_list<VkSubpassDescription> subpasses,
      std::initializer_list<VkSubpassDependency> dependencies) {
    containers::vector<VkAttachmentDescription> attach(allocator_);
    containers::vector<VkSubpassDescription> subpass(allocator_);
    containers::vector<VkSubpassDependency> dep(allocator_);
    attach.insert(attach.begin(), attachments.begin(), attachments.end());
    subpass.insert(subpass.begin(), subpasses.begin(), subpasses.end());
    dep.insert(dep.begin(), dependencies.begin(), dependencies.end());

    VkRenderPassCreateInfo create_info{
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,  // sType
        nullptr,                                    // pNext
        0,                                          // flags
        static_cast<uint32_t>(attach.size()),       // attachmentCount
        attach.size() ? attach.data() : nullptr,    // pAttachments
        static_cast<uint32_t>(subpass.size()),      // subpassCount
        subpass.size() ? subpass.data() : nullptr,  // pSubpasses
        static_cast<uint32_t>(dep.size()),          // dependencyCount
        dep.size() ? dep.data() : nullptr,          // pDependencies
    };

    ::VkRenderPass render_pass;
    LOG_ASSERT(==, log_, VK_SUCCESS,
               device_->vkCreateRenderPass(device_, &create_info, nullptr,
                                           &render_pass));
    return vulkan::VkRenderPass(render_pass, nullptr, &device_);
  }

  VulkanGraphicsPipeline CreateGraphicsPipeline(PipelineLayout* layout,
                                                VkRenderPass* render_pass,
                                                uint32_t subpass_) {
    return VulkanGraphicsPipeline(allocator_, layout, this, render_pass,
                                  subpass_);
  }

  static const VkAccessFlags kAllReadBits =
      VK_ACCESS_HOST_READ_BIT | VK_ACCESS_INDIRECT_COMMAND_READ_BIT |
      VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT |
      VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_MEMORY_READ_BIT |
      VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;

  static const VkAccessFlags kAllWriteBits =
      VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_SHADER_WRITE_BIT |
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
      VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT;

  containers::Allocator* GetAllocator() { return allocator_; }

 private:
  containers::unique_ptr<Buffer> CreateAndBindBuffer(
      VulkanArena* heap, const VkBufferCreateInfo* create_info);

  // Intended to be called by the constructor to create the device, since
  // VkDevice does not have a default constructor.
  VkDevice CreateDevice(const std::initializer_list<const char*> extensions,
                        const VkPhysicalDeviceFeatures& features);

  containers::Allocator* allocator_;
  logging::Logger* log_;
  const entry::entry_data* entry_data_;
  containers::vector<::VkImage> swapchain_images_;
  containers::unique_ptr<VkQueue> render_queue_concrete_;
  containers::unique_ptr<VkQueue> present_queue_concrete_;
  VkQueue* render_queue_;
  VkQueue* present_queue_;
  uint32_t render_queue_index_;
  uint32_t present_queue_index_;

  LibraryWrapper library_wrapper_;
  VkInstance instance_;
  VkSurfaceKHR surface_;
  VkDevice device_;
  VkSwapchainKHR swapchain_;
  VkCommandPool command_pool_;
  VkPipelineCache pipeline_cache_;
  containers::unique_ptr<VulkanArena> host_accessible_heap_;
  containers::unique_ptr<VulkanArena> device_only_image_heap_;
  containers::unique_ptr<VulkanArena> device_only_buffer_heap_;
};

using BufferPointer = containers::unique_ptr<VulkanApplication::Buffer>;
using ImagePointer = containers::unique_ptr<VulkanApplication::Image>;
}  // namespace vulkan

#endif  // VULKAN_HELPERS_VULKAN_APPLICATION
