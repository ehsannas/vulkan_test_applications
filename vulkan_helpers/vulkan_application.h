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
#include "vulkan_wrapper/device_wrapper.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/library_wrapper.h"
#include "vulkan_wrapper/queue_wrapper.h"
#include "vulkan_wrapper/sub_objects.h"

namespace vulkan {

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
    ~Image() { heap_->FreeMemory(token_); }

   private:
    friend class ::vulkan::VulkanApplication;
    Image(VulkanArena* heap, AllocationToken* token, VkImage&& image)
        : heap_(heap), token_(token), image_(std::move(image)) {}
    VulkanArena* heap_;
    AllocationToken* token_;
    VkImage image_;
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

  // Creates and returns a new CommandBuffer using the Applications default
  // VkCommandPool.
  VkCommandBuffer GetCommandBuffer() {
    return CreateDefaultCommandBuffer(&command_pool_, &device_);
  }

  // Returns the Graphics and Compute queue for this application.
  VkQueue& render_queue() { return *render_queue_; }

  // Returns the Present queue for this application. Note: It may be the same
  // as the render queue.
  VkQueue& present_queue() { return *present_queue_; }

  // Returns the device that was created for this application.
  VkDevice& device() { return device_; }

  // Returns true if the Present queue is not the same as the present queue.
  bool HasSeparatePresentQueue() const {
    return present_queue_ != render_queue_;
  }

 private:
  containers::unique_ptr<Buffer> CreateAndBindBuffer(
      VulkanArena* heap, const VkBufferCreateInfo* create_info);

  // Intended to be called by the constructor to create the device, since
  // VkDevice does not have a default constructor.
  VkDevice CreateDevice();

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
  containers::unique_ptr<VulkanArena> host_accessible_heap_;
  containers::unique_ptr<VulkanArena> device_only_image_heap_;
  containers::unique_ptr<VulkanArena> device_only_buffer_heap_;
};

using BufferPointer = containers::unique_ptr<VulkanApplication::Buffer>;
using ImagePointer = containers::unique_ptr<VulkanApplication::Image>;
}  // namespace vulkan

#endif  // VULKAN_HELPERS_VULKAN_APPLICATION
