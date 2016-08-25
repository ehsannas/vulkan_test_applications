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

#include <cstring>

#include "vulkan_wrapper/lazy_function.h"
#include "vulkan_wrapper/library_wrapper.h"

namespace vulkan {

// VkDevice wraps a native vulkan VkDevice handle. It provides
// lazily initialized function pointers for all of its
// methods. It will automatically call VkDestroyDevice when it
// goes out of scope.
class VkDevice;

template <typename T>
using LazyDeviceFunction = LazyFunction<T, ::VkDevice, VkDevice>;

class VkDevice {
public:
// This does not retain a reference to the VkInstance, or the
// VkAllocationCallbacks object, it does take ownership of the device.
#define CONSTRUCT_LAZY_FUNCTION(function) function(device, #function, this)
  VkDevice(::VkDevice device, VkAllocationCallbacks *allocator,
           VkInstance *instance)
      : device_(device), has_allocator_(allocator != nullptr),
        log_(instance->GetLogger()), CONSTRUCT_LAZY_FUNCTION(vkDestroyDevice) {

    if (has_allocator_) {
      allocator_ = *allocator;
    } else {
      memset(&allocator_, 0, sizeof(allocator_));
    }
    vkGetDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(
        instance->get_wrapper()->getProcAddr(*instance, "vkGetDeviceProcAddr"));
    LOG_ASSERT(!=, log_, vkGetDeviceProcAddr,
               static_cast<PFN_vkGetDeviceProcAddr>(nullptr));
  }

  ~VkDevice() {
    if (device_) {
      vkDestroyDevice(device_, has_allocator_ ? &allocator_ : nullptr);
    }
  }

#undef CONSTRUCT_LAZY_FUNCTION

  logging::Logger *GetLogger() { return log_; }

private:
  ::VkDevice device_;
  bool has_allocator_;
  // Intentionally keep a copy of the callbacks, they are just a bunch of
  // pointers, but it means we don't force our user to keep the allocator struct
  // around forever.
  VkAllocationCallbacks allocator_;
  logging::Logger *log_;
  PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;

public:
  PFN_vkVoidFunction getProcAddr(::VkDevice device, const char *function) {
    return vkGetDeviceProcAddr(device, function);
  }
  ::VkDevice get_device() const { return device_; }
  operator ::VkDevice() const { return device_; }

#define LAZY_FUNCTION(function) LazyDeviceFunction<PFN_##function> function;
  LAZY_FUNCTION(vkDestroyDevice);
#undef LAZY_FUNCTION
};

} // namespace vulkan