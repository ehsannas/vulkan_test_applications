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

#pragma once

#include "external/vulkan/vulkan.h"
#include "vulkan_wrapper/library_wrapper.h"

namespace vulkan {

// VkInstance wraps a native vulkan VkInstance handle. It provides
// lazily initialized function pointers for all of its initialization
// methods. It will automatically call VkDestroyInstance when it
// goes out of scope.
class VkInstance {
public:
#define CONSTRUCT_LAZY_FUNCTION(function) function(instance, #function, wrapper)
  VkInstance(::VkInstance instance, VkAllocationCallbacks *allocator,
             LibraryWrapper *wrapper)
      : instance_(instance), has_allocator_(allocator != nullptr),
        wrapper_(wrapper), CONSTRUCT_LAZY_FUNCTION(vkDestroyInstance) {
    if (has_allocator_) {
      allocator_ = *allocator;
    }
  }

  VkInstance(VkInstance &&other)
      : instance_(other.instance_), allocator_(other.allocator_),
        wrapper_(other.wrapper_), has_allocator_(other.has_allocator_),
        vkDestroyInstance(other.vkDestroyInstance) {
    other.instance_ = VK_NULL_HANDLE;
  }

  VkInstance(const VkInstance &) = delete;
  VkInstance &operator=(const VkInstance &) = delete;
#undef CONSTRUCT_LAZY_FUNCTION
  ~VkInstance() {
    if (instance_ != VK_NULL_HANDLE) {
      vkDestroyInstance(instance_, has_allocator_ ? &allocator_ : nullptr);
    }
  }

private:
  ::VkInstance instance_;
  bool has_allocator_;
  // Intentionally keep a copy of the callbacks, they are just a bunch of
  // pointers, but it means we don't force our user to keep the allocator struct
  // around forever.
  VkAllocationCallbacks allocator_;
  LibraryWrapper *wrapper_;

public:
  ::VkInstance get_instance() { return instance_; }
  operator ::VkInstance() { return instance_; }

#define LAZY_FUNCTION(function) LazyInstanceFunction<PFN_##function> function;
  LAZY_FUNCTION(vkDestroyInstance);
#undef LAZY_FUNCTION
};

} // namespace vulkan