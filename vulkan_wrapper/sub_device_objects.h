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

#ifndef VULKAN_WRAPPER_SUB_DEVICE_OBJECTS_H_
#define VULKAN_WRAPPER_SUB_DEVICE_OBJECTS_H_

#include "vulkan_helpers/vulkan_header_wrapper.h"
#include "vulkan_wrapper/device_wrapper.h"
#include "vulkan_wrapper/lazy_function.h"

namespace vulkan {

template <typename T, typename F>
using LazySubDeviceFunction = LazyFunction<T, ::VkDevice, F>;

// VkSubDeviceObject wraps any object that is the child of a device.
// It exists just so it gets cleaned up when the device is destroyed.
// T is expected to be a set of traits that describes the object in question.
// It should be of the form
// struct FooTraits {
//   using type = VulkanType;
//   using destruction_function = PFN_vkDestroyVulkanType;
//   static const char* function_name() const {
//      return "vkDestroyVulkanType";
//   }
//}
template <typename T>
class VkSubDeviceObject {
  using type = typename T::type;

 public:
  // This does not retain a reference to the Device, or the
  // VkAllocationCallbacks object, it does take ownership of the object in
  // question.
  VkSubDeviceObject(type raw_object, VkAllocationCallbacks* allocator,
                    VkDevice* device)
      : device_(*device),
        log_(device->GetLogger()),
        has_allocator_(allocator != nullptr),
        raw_object_(raw_object),
        destruction_function(*device, T::function_name(), this) {
    if (allocator) {
      allocator_ = *allocator;
    }
    vkGetDeviceProcAddr = device->get_device_proc_addr_function();
  }

  ~VkSubDeviceObject() {
    if (raw_object_) {
      destruction_function(device_, raw_object_,
                           has_allocator_ ? &allocator_ : nullptr);
    }
  }

  VkSubDeviceObject(VkSubDeviceObject<T>&& other)
      : device_(other.device_),
        log_(other.log_),
        vkGetDeviceProcAddr(other.vkGetDeviceProcAddr),
        allocator_(other.allocator_),
        has_allocator_(other.has_allocator_),
        raw_object_(other.raw_object_),
        destruction_function(other.destruction_function) {
    other.raw_object_ = VK_NULL_HANDLE;
  }

  logging::Logger* GetLogger() { return log_; }

 private:
  ::VkDevice device_;
  logging::Logger* log_;
  PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
  VkAllocationCallbacks allocator_;
  bool has_allocator_;
  type raw_object_;

  LazySubDeviceFunction<typename T::destruction_function, VkSubDeviceObject<T>>
      destruction_function;

 public:
  operator type() { return raw_object_; }
  type get_raw_object() { return raw_object_; }

  PFN_vkVoidFunction getProcAddr(::VkDevice device, const char* function) {
    return vkGetDeviceProcAddr(device, function);
  }
};

struct CommandPoolTraits {
  using type = ::VkCommandPool;
  using destruction_function = PFN_vkDestroyCommandPool;
  static const char* function_name() { return "vkDestroyCommandPool"; }
};
using VkCommandPool = VkSubDeviceObject<CommandPoolTraits>;

}  // namespace vulkan

#endif  // VULKAN_WRAPPER_SUB_DEVICE_OBJECTS_H_
