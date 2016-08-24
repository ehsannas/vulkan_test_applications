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
#define VK_NO_PROTOTYPES
#include "external/vulkan/vulkan.h"
#undef VK_NO_PROTOTYPES
#include "support/dynamic_loader/dynamic_library.h"
#include "support/log/log.h"

namespace vulkan {

class LibraryWrapper;

// This wraps a lazily initialized function pointer. It will be resolved
// when it is first called.
template <typename T> class LazyInstanceFunction {
public:
// We retain a reference to the function name, so it must remain valid.
// In practice this is expected to be used with string constants.
  LazyInstanceFunction(VkInstance instance, const char *function_name,
                       LibraryWrapper *wrapper)
      : instance_(instance), function_name_(function_name), wrapper_(wrapper) {}

  // When this functor is called, it will check if the function pointer
  // has been resolved. If not it will resolve it and then call the function.
  // If it could not be resolved, the program will segfault.
  template <typename... Args>
  typename std::result_of<T(Args...)>::type operator()(Args... args);

private:
  VkInstance instance_;
  const char *function_name_;
  LibraryWrapper *wrapper_;
  T ptr_ = nullptr;
};

// This wraps the vulkan library. It provides lazily initialized functions
// for all global-scope functions.
class LibraryWrapper {
public:
  LibraryWrapper(logging::Logger *logger);
  bool is_valid() { return vulkan_lib_ && vulkan_lib_->is_valid(); }

#define LAZY_FUNCTION(function)                                                \
  LazyInstanceFunction<PFN_##function> function =                              \
      LazyInstanceFunction<PFN_##function>(nullptr, #function, this)
  LAZY_FUNCTION(vkCreateInstance);
  LAZY_FUNCTION(vkEnumerateInstanceExtensionProperties);
  LAZY_FUNCTION(vkEnumerateInstanceLayerProperties);
#undef LAZY_FUNCTION
private:
  template <typename T> friend class LazyInstanceFunction;

  PFN_vkVoidFunction getInstanceProcAddr(VkInstance instance,
                                         const char *function);
  logging::Logger *GetLogger() { return logger_; }

  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

  logging::Logger *logger_;
  std::unique_ptr<dynamic_loader::DynamicLibrary> vulkan_lib_;
};

template <typename T>
template <typename... Args>
typename std::result_of<T(Args...)>::type LazyInstanceFunction<T>::
operator()(Args... args) {
  if (!ptr_) {
    ptr_ = reinterpret_cast<T>(
        wrapper_->getInstanceProcAddr(instance_, function_name_));
    if (ptr_) {
      wrapper_->GetLogger()->LogInfo(function_name_, " for instance ",
                                     instance_, " resolved");
    } else {
      wrapper_->GetLogger()->LogError(function_name_, " for instance ",
                                      instance_,
                                      " could not be resolved, crashing now");
    }
  }
  return ptr_(args...);
}
}