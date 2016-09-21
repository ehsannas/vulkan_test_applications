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

#ifndef VULKAN_WRAPPER_SUB_OBJECTS_H_
#define VULKAN_WRAPPER_SUB_OBJECTS_H_

#include "vulkan_helpers/vulkan_header_wrapper.h"
#include "vulkan_wrapper/device_wrapper.h"
#include "vulkan_wrapper/instance_wrapper.h"
#include "vulkan_wrapper/lazy_function.h"

namespace vulkan {

// VkSubObject wraps any object that is the child of an instance or device.
// It exists just so it gets cleaned up when it goes out of scope.
// T is expected to be a set of traits that describes the object in question.
// It should be of the form
// struct FooTraits {
//   using type = VulkanType;
//   using destruction_function_type =
//     Lazy{Instance|Device|..}Function<PFN_vkDestoryVulkanType>;
//   static destruction_function_type* get_destruction_function(
//       {Device|Instance|...}Functions* functions) {
//     return &functions->vkDestroyVulkanType;
//   }
// }
// O is expected to be a set of traits that describe the owner of this object.
// It should be of the form
// struct OwnerTraits {
//   using type = VulkanType; // Typically VkDevice or VkInstance
//   using proc_addr_function_type; // PFN_vkGetInstanceProcAddr for example
//   using raw_vulkan_type; // Raw vulkan type that this is associated with
// }
//}
template <typename T, typename O>
class VkSubObject {
  using type = typename T::type;
  using owner_type = typename O::type;
  using raw_owner_type = typename O::raw_vulkan_type;
  using proc_addr_type = typename O::proc_addr_function_type;
  using destruction_function_type = typename T::destruction_function_type;

 public:
  // This does not retain a reference to the owner, or the
  // VkAllocationCallbacks object, it does take ownership of the object in
  // question.
  VkSubObject(type raw_object, VkAllocationCallbacks* allocator,
              owner_type* owner)
      : owner_(*owner),
        log_(owner->GetLogger()),
        has_allocator_(allocator != nullptr),
        raw_object_(raw_object),
        destruction_function_(T::get_destruction_function(owner->functions())) {
    if (allocator) {
      allocator_ = *allocator;
    }
    get_proc_addr_fn_ = owner->getProcAddrFunction();
  }

  ~VkSubObject() {
    if (raw_object_) {
      (*destruction_function_)(owner_, raw_object_,
                               has_allocator_ ? &allocator_ : nullptr);
    }
  }

  VkSubObject(VkSubObject<T, O>&& other)
      : owner_(other.owner_),
        log_(other.log_),
        get_proc_addr_fn_(other.get_proc_addr_fn_),
        allocator_(other.allocator_),
        has_allocator_(other.has_allocator_),
        raw_object_(other.raw_object_),
        destruction_function_(other.destruction_function_) {
    other.raw_object_ = VK_NULL_HANDLE;
  }

  logging::Logger* GetLogger() { return log_; }

 private:
  raw_owner_type owner_;
  logging::Logger* log_;
  proc_addr_type get_proc_addr_fn_;
  VkAllocationCallbacks allocator_;
  bool has_allocator_;
  type raw_object_;

  destruction_function_type* destruction_function_;

 public:
  operator type() const { return raw_object_; }
  type get_raw_object() { return raw_object_; }

  PFN_vkVoidFunction getProcAddr(raw_owner_type owner, const char* function) {
    return get_proc_addr_fn_(owner, function);
  }
};

struct InstanceTraits {
  using type = VkInstance;
  using proc_addr_function_type = PFN_vkGetInstanceProcAddr;
  using raw_vulkan_type = ::VkInstance;
  using function_table_type = InstanceFunctions;
};

struct DeviceTraits {
  using type = VkDevice;
  using proc_addr_function_type = PFN_vkGetDeviceProcAddr;
  using raw_vulkan_type = ::VkDevice;
};

struct CommandPoolTraits {
  using type = ::VkCommandPool;
  using destruction_function_type =
      LazyDeviceFunction<PFN_vkDestroyCommandPool>;
  static destruction_function_type* get_destruction_function(
      DeviceFunctions* functions) {
    return &functions->vkDestroyCommandPool;
  }
};
using VkCommandPool = VkSubObject<CommandPoolTraits, DeviceTraits>;

struct SurfaceTraits {
  using type = ::VkSurfaceKHR;
  using destruction_function_type =
      LazyInstanceFunction<PFN_vkDestroySurfaceKHR>;
  static destruction_function_type* get_destruction_function(
      InstanceFunctions* functions) {
    return &functions->vkDestroySurfaceKHR;
  }
};
using VkSurfaceKHR = VkSubObject<SurfaceTraits, InstanceTraits>;

struct SwapchainTraits {
  using type = ::VkSwapchainKHR;
  using destruction_function_type =
      LazyDeviceFunction<PFN_vkDestroySwapchainKHR>;
  static destruction_function_type* get_destruction_function(
      DeviceFunctions* functions) {
    return &functions->vkDestroySwapchainKHR;
  }
};
using VkSwapchainKHR = VkSubObject<SwapchainTraits, DeviceTraits>;

}  // namespace vulkan

#endif  // VULKAN_WRAPPER_SUB_OBJECTS_H_