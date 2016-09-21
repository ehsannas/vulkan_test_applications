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

#ifndef VULKAN_WRAPPER_FUNCTION_TABLE_H_
#define VULKAN_WRAPPER_FUNCTION_TABLE_H_

#include "vulkan_wrapper/lazy_function.h"

namespace vulkan {

class InstanceFunctions;
template <typename T>
using LazyInstanceFunction = LazyFunction<T, ::VkInstance, InstanceFunctions>;

// InstanceFunctions contains a list of lazily resolved Vulkan instance
// functions. All the lazily resolved functions are implemented through
// LazyFunction template, GetLogger() and getProcAddr() methods are required to
// conform the LazyFunction template. As this class is the source of lazily
// resolved Vulkan functions, the instance of this class is non-movable and
// non-copyable.
class InstanceFunctions {
 public:
  InstanceFunctions(const InstanceFunctions& other) = delete;
  InstanceFunctions(InstanceFunctions&& other) = delete;
  InstanceFunctions& operator=(const InstanceFunctions& other) = delete;
  InstanceFunctions& operator=(InstanceFunctions&& other) = delete;

  InstanceFunctions(::VkInstance instance,
                    PFN_vkGetInstanceProcAddr get_proc_addr_func,
                    logging::Logger* log)
      : log_(log),
        vkGetInstanceProcAddr_(get_proc_addr_func),
#define CONSTRUCT_LAZY_FUNCTION(function) function(instance, #function, this)
        CONSTRUCT_LAZY_FUNCTION(vkDestroyInstance),
        CONSTRUCT_LAZY_FUNCTION(vkEnumeratePhysicalDevices),
        CONSTRUCT_LAZY_FUNCTION(vkCreateDevice),
        CONSTRUCT_LAZY_FUNCTION(vkEnumerateDeviceExtensionProperties),
        CONSTRUCT_LAZY_FUNCTION(vkEnumerateDeviceLayerProperties),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceFeatures),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceMemoryProperties),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceProperties),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceFormatProperties),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceImageFormatProperties),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceSparseImageFormatProperties),
        CONSTRUCT_LAZY_FUNCTION(vkDestroySurfaceKHR),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR),
        CONSTRUCT_LAZY_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR)
#if defined __ANDROID__
        ,
        CONSTRUCT_LAZY_FUNCTION(vkCreateAndroidSurfaceKHR)
#elif defined __linux__
        ,
        CONSTRUCT_LAZY_FUNCTION(vkCreateXcbSurfaceKHR)
#elif defined __WIN32__
        ,
        CONSTRUCT_LAZY_FUNCTION(vkCreateWin32SurfaceKHR)
#endif
#undef CONSTRUCT_LAZY_FUNCTION
  {
  }

 private:
  logging::Logger* log_;
  // The function pointer to Vulkan vkGetInstanceProcAddr().
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr_;

 public:
  // Returns the logger. This is required to conform LazyFunction template.
  logging::Logger* GetLogger() { return log_; }
  // Resolves an instance function with the given name. This is required to
  // conform LazyFunction template.
  PFN_vkVoidFunction getProcAddr(::VkInstance instance, const char* function) {
    return vkGetInstanceProcAddr_(instance, function);
  }

#define LAZY_FUNCTION(function) LazyInstanceFunction<PFN_##function> function;
  LAZY_FUNCTION(vkDestroyInstance);
  LAZY_FUNCTION(vkEnumeratePhysicalDevices);
  LAZY_FUNCTION(vkCreateDevice);
  LAZY_FUNCTION(vkEnumerateDeviceExtensionProperties);
  LAZY_FUNCTION(vkEnumerateDeviceLayerProperties);
  LAZY_FUNCTION(vkGetPhysicalDeviceFeatures);
  LAZY_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
  LAZY_FUNCTION(vkGetPhysicalDeviceProperties);
  LAZY_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
  LAZY_FUNCTION(vkGetPhysicalDeviceFormatProperties);
  LAZY_FUNCTION(vkGetPhysicalDeviceImageFormatProperties);
  LAZY_FUNCTION(vkGetPhysicalDeviceSparseImageFormatProperties);
  LAZY_FUNCTION(vkDestroySurfaceKHR);
  LAZY_FUNCTION(vkGetPhysicalDeviceSurfaceSupportKHR);
  LAZY_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
  LAZY_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
  LAZY_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);
#if defined __ANDROID__
  LAZY_FUNCTION(vkCreateAndroidSurfaceKHR);
#elif defined __linux__
  LAZY_FUNCTION(vkCreateXcbSurfaceKHR);
#elif defined __WIN32__
  LAZY_FUNCTION(vkCreateWin32SurfaceKHR);
#endif
#undef LAZY_FUNCTION
};

class DeviceFunctions;
template <typename T>
using LazyDeviceFunction = LazyFunction<T, ::VkDevice, DeviceFunctions>;

// CommandBufferFunctions stores a list of lazily resolved Vulkan Command
// buffer functions. The instance of this class should be owned and the
// functions listed inside should be resolved by DeviceFunctions. This class
// does not need to conform LazyFunction template as no function is resolved
// through it.
struct CommandBufferFunctions {
 public:
  CommandBufferFunctions(::VkDevice device, DeviceFunctions* device_functions)
      :
#define CONSTRUCT_LAZY_FUNCTION(function) \
  function(device, #function, device_functions)
        CONSTRUCT_LAZY_FUNCTION(vkBeginCommandBuffer),
        CONSTRUCT_LAZY_FUNCTION(vkEndCommandBuffer),
        CONSTRUCT_LAZY_FUNCTION(vkResetCommandBuffer)
#undef CONSTRUCT_LAZY_FUNCTION
  {
  }

 public:
#define LAZY_FUNCTION(function) LazyDeviceFunction<PFN_##function> function;
  LAZY_FUNCTION(vkBeginCommandBuffer);
  LAZY_FUNCTION(vkEndCommandBuffer);
  LAZY_FUNCTION(vkResetCommandBuffer);
#undef LAZY_FUNCTION
};

// DeviceFunctions contains a list of lazily resolved Vulkan device functions
// and the functions of sub-device objects.All the lazily resolved functions
// are implemented through LazyFunction template, GetLogger() and getProcAddr()
// methods are required to conform the LazyFunction template. As this class is
// the source of lazily resolved Vulkan functions, the instance of this class
// is non-movable and non-copyable.
class DeviceFunctions {
 public:
  DeviceFunctions(const DeviceFunctions& other) = delete;
  DeviceFunctions(DeviceFunctions&& other) = delete;
  DeviceFunctions& operator=(const DeviceFunctions& other) = delete;
  DeviceFunctions& operator=(DeviceFunctions&& other) = delete;

  DeviceFunctions(::VkDevice device, PFN_vkGetDeviceProcAddr get_proc_addr_func,
                  logging::Logger* log)
      : log_(log),
        vkGetDeviceProcAddr_(get_proc_addr_func),
        command_buffer_functions_(device, this),
#define CONSTRUCT_LAZY_FUNCTION(function) function(device, #function, this)
        CONSTRUCT_LAZY_FUNCTION(vkDestroyDevice),
        CONSTRUCT_LAZY_FUNCTION(vkCreateCommandPool),
        CONSTRUCT_LAZY_FUNCTION(vkDestroyCommandPool),
        CONSTRUCT_LAZY_FUNCTION(vkAllocateCommandBuffers),
        CONSTRUCT_LAZY_FUNCTION(vkFreeCommandBuffers),
        CONSTRUCT_LAZY_FUNCTION(vkGetDeviceQueue),
        CONSTRUCT_LAZY_FUNCTION(vkCreateSemaphore),
        CONSTRUCT_LAZY_FUNCTION(vkDestroySemaphore),
        CONSTRUCT_LAZY_FUNCTION(vkCreateImage),
        CONSTRUCT_LAZY_FUNCTION(vkDestroyImage),
        CONSTRUCT_LAZY_FUNCTION(vkCreateSwapchainKHR),
        CONSTRUCT_LAZY_FUNCTION(vkDestroySwapchainKHR),
        CONSTRUCT_LAZY_FUNCTION(vkGetSwapchainImagesKHR)
#undef CONSTRUCT_LAZY_FUNCTION
  {
  }

 private:
  logging::Logger* log_;
  // The function pointer to Vulkan vkGetDeviceProcAddr().
  PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr_;
  // Functions of sub device objects.
  CommandBufferFunctions command_buffer_functions_;

 public:
  // Returns the logger. This is required to conform LazyFunction template.
  logging::Logger* GetLogger() { return log_; }
  // Resolves a device function with the given name. This is required to
  // conform LazyFunction template.
  PFN_vkVoidFunction getProcAddr(::VkDevice device, const char* function) {
    return vkGetDeviceProcAddr_(device, function);
  }
  // Access the command buffer functions.
  CommandBufferFunctions* command_buffer_functions() {
    return &command_buffer_functions_;
  }

#define LAZY_FUNCTION(function) LazyDeviceFunction<PFN_##function> function;
  LAZY_FUNCTION(vkDestroyDevice);
  LAZY_FUNCTION(vkCreateCommandPool);
  LAZY_FUNCTION(vkDestroyCommandPool);
  LAZY_FUNCTION(vkAllocateCommandBuffers);
  LAZY_FUNCTION(vkFreeCommandBuffers);
  LAZY_FUNCTION(vkGetDeviceQueue);
  LAZY_FUNCTION(vkCreateSemaphore);
  LAZY_FUNCTION(vkDestroySemaphore);
  LAZY_FUNCTION(vkCreateImage);
  LAZY_FUNCTION(vkDestroyImage);
  LAZY_FUNCTION(vkCreateSwapchainKHR);
  LAZY_FUNCTION(vkDestroySwapchainKHR);
  LAZY_FUNCTION(vkGetSwapchainImagesKHR);
#undef LAZY_FUNCTION
};

}  // namespace vulkan

#endif  // VULKAN_WRAPPER_FUNCTION_TABLE_H_
