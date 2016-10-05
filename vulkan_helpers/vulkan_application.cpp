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

#include "vulkan_helpers/vulkan_application.h"
#include "vulkan_helpers/helper_functions.h"

namespace vulkan {

VulkanApplication::VulkanApplication(containers::Allocator* allocator,
                                     logging::Logger* log,
                                     const entry::entry_data* entry_data,
                                     uint32_t host_buffer_size,
                                     uint32_t device_image_size,
                                     uint32_t device_buffer_size)
    : allocator_(allocator),
      log_(log),
      entry_data_(entry_data),
      swapchain_images_(allocator_),
      render_queue_(nullptr),
      present_queue_(nullptr),
      render_queue_index_(0u),
      present_queue_index_(0u),
      library_wrapper_(allocator_, log_),
      instance_(CreateDefaultInstance(allocator_, &library_wrapper_)),
      surface_(CreateDefaultSurface(&instance_, entry_data_)),
      device_(CreateDevice()),
      swapchain_(CreateDefaultSwapchain(&instance_, &device_, &surface_,
                                        allocator_, render_queue_index_,
                                        present_queue_index_)),
      command_pool_(CreateDefaultCommandPool(allocator_, device_)) {
  vulkan::LoadContainer(log_, device_->vkGetSwapchainImagesKHR,
                        &swapchain_images_, device_, swapchain_);
  // Relevant spec sections for determining what memory we will be allowed
  // to use for our buffer allocations.
  //  The memoryTypeBits member is identical for all VkBuffer objects created
  //  with the same value for the flags and usage members in the
  //  VkBufferCreateInfo structure passed to vkCreateBuffer. Further, if usage1
  //  and usage2 of type VkBufferUsageFlags are such that the bits set in usage2
  //  are a subset of the bits set in usage1, and they have the same flags,
  //  then the bits set in memoryTypeBits returned for usage1 must be a subset
  //  of
  //  the bits set in memoryTypeBits returned for usage2, for all values of
  //  flags.

  // Therefore we should be able to satisfy all buffer requests for non
  // sparse memory bindings if we do the following:
  // For our host visible bits, we will use:
  // VK_BUFFER_USAGE_TRANSFER_SRC_BIT
  // VK_BUFFER_USAGE_TRANSFER_DST_BIT
  // For our device buffers we will use ALL bits.
  // This means we can use this memory for everything.
  // Furthermore for both types, we will have ZERO flags
  // set (we do not want to do sparse binding.)

  containers::unique_ptr<VulkanArena>* device_memories[2] = {
      &host_accessible_heap_, &device_only_buffer_heap_};
  uint32_t device_memory_sizes[2] = {host_buffer_size, device_buffer_size};

  const uint32_t kAllBufferBits =
      (VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT << 1) - 1;

  VkBufferUsageFlags usages[2] = {
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      kAllBufferBits};
  VkMemoryPropertyFlags property_flags[2] = {
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT};

  for (size_t i = 0; i < 2; ++i) {
    // 1) Create a tiny buffer so that we can determine what memory flags are
    // required.
    VkBufferCreateInfo create_info = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,  // sType
        nullptr,                               // pNext
        0,                                     // flags
        1,                                     // size
        usages[i],                             // usage
        VK_SHARING_MODE_EXCLUSIVE,             // sharingMode
        0,                                     // queueFamilyIndexCount
        nullptr,                               //  pQueueFamilyIndices
    };
    ::VkBuffer buffer;
    LOG_ASSERT(==, log_,
               device_->vkCreateBuffer(device_, &create_info, nullptr, &buffer),
               VK_SUCCESS);
    // Get the memory requirements for this buffer.
    VkMemoryRequirements requirements;
    device_->vkGetBufferMemoryRequirements(device_, buffer, &requirements);
    device_->vkDestroyBuffer(device_, buffer, nullptr);

    uint32_t memory_index = GetMemoryIndex(
        &device_, log_, requirements.memoryTypeBits, property_flags[i]);
    *device_memories[i] = containers::make_unique<VulkanArena>(
        allocator_, allocator_, log_, device_memory_sizes[i], memory_index,
        &device_, property_flags[i] & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
  }

  // Same idea as above, but for image memory.
  // The relevant bits from the spec are:
  //  The memoryTypeBits member is identical for all VkImage objects created
  //  with the same combination of values for the tiling member and the
  //  VK_IMAGE_CREATE_SPARSE_BINDING_BIT bit of the flags member and the
  //  VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT of the usage member in the
  //  VkImageCreateInfo structure passed to vkCreateImage.
  {
    VkImageCreateInfo image_create_info{
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,  // sType
        nullptr,                              // pNext
        0,                                    // flags
        VK_IMAGE_TYPE_2D,                     // imageType
        VK_FORMAT_R8G8B8A8_UNORM,             // format
        {
            // extent
            1,  // width
            1,  // height
            1,  // depth
        },
        1,                                    // mipLevels
        1,                                    // arrayLayers
        VK_SAMPLE_COUNT_1_BIT,                // samples
        VK_IMAGE_TILING_OPTIMAL,              // tiling
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  // usage
        VK_SHARING_MODE_EXCLUSIVE,            // sharingMode
        0,                                    // queueFamilyIndexCount
        nullptr,                              // pQueueFamilyIndices
        VK_IMAGE_LAYOUT_UNDEFINED,            // initialLayout
    };
    ::VkImage image;
    LOG_ASSERT(==, log_, device_->vkCreateImage(device_, &image_create_info,
                                                nullptr, &image),
               VK_SUCCESS);
    VkMemoryRequirements requirements;
    device_->vkGetImageMemoryRequirements(device_, image, &requirements);
    device_->vkDestroyImage(device_, image, nullptr);

    uint32_t memory_index =
        GetMemoryIndex(&device_, log_, requirements.memoryTypeBits,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    device_only_image_heap_ = containers::make_unique<VulkanArena>(
        allocator_, allocator_, log_, device_image_size, memory_index, &device_,
        false);
  }
}

VkDevice VulkanApplication::CreateDevice() {
  // Since this is called by the constructor be careful not to
  // use any data other than what has already been initialized.
  // allocator_, log_, entry_data_, library_wrapper_, instance_,
  // surface_

  vulkan::VkDevice device(vulkan::CreateDeviceForSwapchain(
      allocator_, &instance_, &surface_, &render_queue_index_,
      &present_queue_index_));
  if (render_queue_index_ == present_queue_index_) {
    render_queue_concrete_ = containers::make_unique<VkQueue>(
        allocator_, GetQueue(&device, render_queue_index_));
    render_queue_ = render_queue_concrete_.get();
    present_queue_ = render_queue_concrete_.get();
  } else {
    render_queue_concrete_ = containers::make_unique<VkQueue>(
        allocator_, GetQueue(&device, render_queue_index_));
    present_queue_concrete_ = containers::make_unique<VkQueue>(
        allocator_, GetQueue(&device, present_queue_index_));
    render_queue_ = render_queue_concrete_.get();
    present_queue_ = present_queue_concrete_.get();
  }
  return std::move(device);
}

containers::unique_ptr<VulkanApplication::Image>
VulkanApplication::CreateAndBindImage(const VkImageCreateInfo* create_info) {
  ::VkImage image;
  LOG_ASSERT(==, log_,
             device_->vkCreateImage(device_, create_info, nullptr, &image),
             VK_SUCCESS);
  VkMemoryRequirements requirements;
  device_->vkGetImageMemoryRequirements(device_, image, &requirements);

  ::VkDeviceMemory memory;
  ::VkDeviceSize offset;

  AllocationToken* token = device_only_image_heap_->AllocateMemory(
      requirements.size, requirements.alignment, &memory, &offset, nullptr);

  device_->vkBindImageMemory(device_, image, memory, offset);

  // We have to do it this way because Image is private and friended,
  // so we cannot go through make_unique.
  Image* img = new (allocator_->malloc(sizeof(Image))) Image(
      device_only_image_heap_.get(), token, VkImage(image, nullptr, &device_));

  return containers::unique_ptr<Image>(
      img, containers::UniqueDeleter(allocator_, sizeof(Image)));
}

containers::unique_ptr<VulkanApplication::Buffer>
VulkanApplication::CreateAndBindBuffer(VulkanArena* heap,
                                       const VkBufferCreateInfo* create_info) {
  ::VkBuffer buffer;
  LOG_ASSERT(==, log_,
             device_->vkCreateBuffer(device_, create_info, nullptr, &buffer),
             VK_SUCCESS);
  // Get the memory requirements for this buffer.
  VkMemoryRequirements requirements;
  device_->vkGetBufferMemoryRequirements(device_, buffer, &requirements);
  ::VkDeviceMemory memory;
  ::VkDeviceSize offset;
  char* base_address;

  AllocationToken* token =
      heap->AllocateMemory(requirements.size, requirements.alignment, &memory,
                           &offset, &base_address);

  device_->vkBindBufferMemory(device_, buffer, memory, offset);

  Buffer* buff = new (allocator_->malloc(sizeof(Buffer))) Buffer(
      heap, token, VkBuffer(buffer, nullptr, &device_), base_address, device_,
      memory, offset, requirements.size, &(device_->vkFlushMappedMemoryRanges),
      &(device_->vkInvalidateMappedMemoryRanges));
  return containers::unique_ptr<Buffer>(
      buff, containers::UniqueDeleter(allocator_, sizeof(Buffer)));
}

containers::unique_ptr<VulkanApplication::Buffer>
VulkanApplication::CreateAndBindHostBuffer(
    const VkBufferCreateInfo* create_info) {
  return CreateAndBindBuffer(host_accessible_heap_.get(), create_info);
}

containers::unique_ptr<VulkanApplication::Buffer>
VulkanApplication::CreateAndBindDeviceBuffer(
    const VkBufferCreateInfo* create_info) {
  return CreateAndBindBuffer(device_only_buffer_heap_.get(), create_info);
}

// These linked-list nodes are ordered by offset into the heap.
// the first node has a prev of nullptr, and the last node has a next of
// nullptr.
struct AllocationToken {
  AllocationToken* next;
  AllocationToken* prev;
  ::VkDeviceSize allocationSize;
  ::VkDeviceSize offset;
  // Location into the map of unused chunks. This is only valid when
  // in_use == false.
  containers::ordered_multimap<::VkDeviceSize, AllocationToken*>::iterator
      map_location;
  bool in_use;
};

VulkanArena::VulkanArena(containers::Allocator* allocator, logging::Logger* log,
                         ::VkDeviceSize buffer_size, uint32_t memory_type_index,
                         VkDevice* device, bool map)
    : allocator_(allocator),
      freeblocks_(allocator_),
      first_block_(nullptr),
      base_address_(nullptr),
      device_(*device),
      unmap_memory_function_(nullptr),
      memory_(VK_NULL_HANDLE, nullptr, device),
      log_(log) {
  // Actually allocate the bytes for this heap.
  VkMemoryAllocateInfo allocate_info{
      VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,  // sType
      nullptr,                                 // pNext
      buffer_size,                             // allocationSize
      memory_type_index};

  ::VkDeviceMemory device_memory;
  LOG_ASSERT(==, log, VK_SUCCESS,
             (*device)->vkAllocateMemory(*device, &allocate_info, nullptr,
                                         &device_memory));
  memory_.initialize(device_memory);

  // Create a new pointer that is the first block of memory. It contains
  // all of the memory in the arena.
  first_block_ = allocator_->construct<AllocationToken>(AllocationToken{
      nullptr, nullptr, buffer_size, 0, freeblocks_.end(), false});

  // Since this has not been used yet, add it to our freeblock_ map.
  first_block_->map_location =
      freeblocks_.insert(std::make_pair(buffer_size, first_block_));

  if (map) {
    // If we were asked to map this memory. (i.e. it is meant to be host
    // visible), then do it now.
    LOG_ASSERT(
        ==, log, VK_SUCCESS,
        (*device)->vkMapMemory(*device, memory_, 0, buffer_size, 0,
                               reinterpret_cast<void**>(&base_address_)));
    // Store off the devices unmap memory function for the future, we only
    // want to keep a reference to the raw device, and not the vulkan::VkDevice
    // since vulkan::VkDevice is movable.
    unmap_memory_function_ = &(*device)->vkUnmapMemory;
  }
}

VulkanArena::~VulkanArena() {
  // Make sure that there is only one block left, and that is is not in use.
  // This will trigger if someone has not freed all the memory before the
  // heap has been destroyed.
  LOG_ASSERT(==, log_, true, first_block_->next == nullptr);
  LOG_ASSERT(==, log_, false, first_block_->in_use);
  if (base_address_) {
    (*unmap_memory_function_)(device_, memory_);
  }
  allocator_->free(first_block_, sizeof(AllocationToken));
}

AllocationToken* VulkanArena::AllocateMemory(::VkDeviceSize size,
                                             ::VkDeviceSize alignment,
                                             ::VkDeviceMemory* memory,
                                             ::VkDeviceSize* offset,
                                             char** base_address) {
  // We use alignment - 1 quite a bit, so store it off here.
  const ::VkDeviceSize align_m_1 = alignment - 1;
  LOG_ASSERT(>, log_, alignment, 0);  // Alignment must be > 0
  LOG_ASSERT(==, log_, !(alignment & (align_m_1)),
             true);  // Alignment must be power of 2.

  // This is the maximum amount of memory we will potentially have to
  // allocate in order to satisfy the alignment.
  ::VkDeviceSize to_allocate = size + align_m_1;

  // Find a block that contains at LEAST enough memory for our allocation.
  auto it = freeblocks_.lower_bound(to_allocate);
  // Fail if there is not even a single free-block that can hold our allocation.
  LOG_ASSERT(==, log_, true, freeblocks_.end() != it);

  AllocationToken* token = it->second;
  // Remove the block that we found from the freeblock map.
  freeblocks_.erase(it);

  // total_offset is the offset from the base of the entire arena to the
  // correctly aligned base inside of the given block.
  ::VkDeviceSize total_offset = (token->offset + (align_m_1)) & ~(align_m_1);
  // offset_from_start is the offset from the start of the block to
  // the alignment location.
  ::VkDeviceSize offset_from_start = total_offset - token->offset;

  // Our block may satisfy the alignment already, so only actually allocate
  // the amount of memory we need.
  // TODO(awoloszyn): If we find fragmentation to be a problem here, then
  //   eventually actually allocate the total. If we do not do this,
  //   then if we have (for example) a 128byte aligned block and we need
  //   4K of memory, we wont be able to re-use this block for another 4K
  //   allocation.
  ::VkDeviceSize total_allocated =
      to_allocate - (align_m_1 - offset_from_start);

  // Remove the memory from the block.
  // Push the block's base up by the allocated memory
  token->allocationSize -= total_allocated;
  token->offset += total_allocated;

  // Create a new block that contains the memory in question.
  AllocationToken* new_token = allocator_->construct<AllocationToken>(
      AllocationToken{nullptr, token->prev, total_allocated, total_offset,
                      freeblocks_.end(), true});

  if (token->allocationSize > 0) {
    // If there is still some space in this allocation, put it back, so we can
    // get more out of it later.
    token->map_location =
        freeblocks_.insert(std::make_pair(token->allocationSize, token));

    // Hook up all of our linked-list nodes.
    new_token->next = token;
    if (!token->prev) {
      first_block_ = new_token;
    } else {
      new_token->prev = token->prev;
      new_token->prev->next = new_token;
    }
    token->prev = new_token;
    new_token->next = token;
  } else {
    // token happens to now be an empty block. So let's not put it back.
    if (token->next) {
      new_token->next = token->next;
      token->next->prev = new_token;
    }
    if (token->prev) {
      token->prev->next = new_token;
    } else {
      first_block_ = new_token;
    }

    allocator_->free(token, sizeof(AllocationToken));
  }
  *memory = memory_;
  *offset = total_offset;
  if (base_address) {
    *base_address = base_address_ ? base_address_ + total_offset : nullptr;
  }
  return new_token;
}

void VulkanArena::FreeMemory(AllocationToken* token) {
  // First try to coalesce this with its previous block.
  while (token->prev && !token->prev->in_use) {
    // Take the previous token out of the map, and merge it with this one.
    AllocationToken* prev_token = token->prev;
    prev_token->allocationSize += token->allocationSize;
    prev_token->next = token->next;
    if (token->next) {
      token->next->prev = prev_token;
    }
    // Remove the previous block from freeblocks_,
    // we have now merged with it.
    freeblocks_.erase(prev_token->map_location);
    allocator_->free(token, sizeof(AllocationToken));
    token = prev_token;
  }
  // Now try to coalesce this with any subsequent blocks.
  while (token->next && !token->next->in_use) {
    // Take the previous token out of the map, and merge it with this one.
    AllocationToken* next_token = token->next;
    token->allocationSize += next_token->allocationSize;
    token->next = next_token->next;
    if (token->next) {
      token->next->prev = token;
    }
    // Remove the next block from freeblocks_,
    // we have now merged with it.
    freeblocks_.erase(next_token->map_location);
    allocator_->free(next_token, sizeof(AllocationToken));
  }
  // This block is no longer being used.
  token->in_use = false;
  // Push it back into freeblocks_.
  token->map_location =
      freeblocks_.insert(std::make_pair(token->allocationSize, token));
}
}