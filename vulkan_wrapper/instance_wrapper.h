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

#ifndef VULKAN_WRAPPER_INSTANCE_WRAPPER_H_
#define VULKAN_WRAPPER_INSTANCE_WRAPPER_H_

#include "external/vulkan/vulkan.h"

#include <cstring>

#include "vulkan_wrapper/lazy_function.h"
#include "vulkan_wrapper/library_wrapper.h"

namespace vulkan {

// VkInstance wraps a native vulkan VkInstance handle. It provides
// lazily initialized function pointers for all of its initialization
// methods. It will automatically call VkDestroyInstance when it
// goes out of scope.
class VkInstance {
 public:
  VkInstance(::VkInstance instance, VkAllocationCallbacks* allocator,
             LibraryWrapper* wrapper)
      : instance_(instance),
        has_allocator_(allocator != nullptr),
        wrapper_(wrapper),
#define CONSTRUCT_LAZY_FUNCTION(function) function(instance, #function, wrapper)
        CONSTRUCT_LAZY_FUNCTION(vkDestroyInstance),
        CONSTRUCT_LAZY_FUNCTION(vkEnumeratePhysicalDevices),
        CONSTRUCT_LAZY_FUNCTION(vkCreateDevice),
        CONSTRUCT_LAZY_FUNCTION(vkEnumerateDeviceExtensionProperties),
        CONSTRUCT_LAZY_FUNCTION(vkEnumerateDeviceLayerProperties),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceFeatures),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceMemoryProperties),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceProperties)
#undef CONSTRUCT_LAZY_FUNCTION
  {
    if (has_allocator_) {
      allocator_ = *allocator;
    } else {
      memset(&allocator_, 0, sizeof(allocator_));
    }
  }

  VkInstance(VkInstance&& other)
      : instance_(other.instance_),
        allocator_(other.allocator_),
        wrapper_(other.wrapper_),
        has_allocator_(other.has_allocator_),
#define MOVE_LAZY_FUNCTION(function) function(other.function)
        MOVE_LAZY_FUNCTION(vkDestroyInstance),
        MOVE_LAZY_FUNCTION(vkEnumeratePhysicalDevices),
        MOVE_LAZY_FUNCTION(vkCreateDevice),
        MOVE_LAZY_FUNCTION(vkEnumerateDeviceExtensionProperties),
        MOVE_LAZY_FUNCTION(vkEnumerateDeviceLayerProperties),
        MOVE_LAZY_FUNCTION(vkGetPhysicalDeviceFeatures),
        MOVE_LAZY_FUNCTION(vkGetPhysicalDeviceMemoryProperties),
        MOVE_LAZY_FUNCTION(vkGetPhysicalDeviceProperties)
#undef COPY_LAZY_FUNCTION
  {
    other.instance_ = VK_NULL_HANDLE;
  }

  VkInstance(const VkInstance&) = delete;
  VkInstance& operator=(const VkInstance&) = delete;

  ~VkInstance() {
    if (instance_ != VK_NULL_HANDLE) {
      vkDestroyInstance(instance_, has_allocator_ ? &allocator_ : nullptr);
    }
  }

  logging::Logger* GetLogger() { return wrapper_->GetLogger(); }
  LibraryWrapper* get_wrapper() { return wrapper_; }

 private:
  ::VkInstance instance_;
  bool has_allocator_;
  // Intentionally keep a copy of the callbacks, they are just a bunch of
  // pointers, but it means we don't force our user to keep the allocator struct
  // around forever.
  VkAllocationCallbacks allocator_;
  LibraryWrapper* wrapper_;

 public:
  ::VkInstance get_instance() const { return instance_; }
  operator ::VkInstance() const { return instance_; }

#define LAZY_FUNCTION(function) LazyInstanceFunction<PFN_##function> function;
  LAZY_FUNCTION(vkDestroyInstance);
  LAZY_FUNCTION(vkEnumeratePhysicalDevices);
  LAZY_FUNCTION(vkCreateDevice);
  LAZY_FUNCTION(vkEnumerateDeviceExtensionProperties);
  LAZY_FUNCTION(vkEnumerateDeviceLayerProperties);
  LAZY_FUNCTION(vkGetPhysicalDeviceFeatures);
  LAZY_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
  LAZY_FUNCTION(vkGetPhysicalDeviceProperties);
#undef LAZY_FUNCTION
};

}  // namespace vulkan

#endif  // VULKAN_WRAPPER_INSTANCE_WRAPPER_H_
