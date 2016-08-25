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

#include "vulkan_wrapper/lazy_function.h"

namespace vulkan {

class LibraryWrapper;
template <typename T>
using LazyInstanceFunction = LazyFunction<T, ::VkInstance, LibraryWrapper>;
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
  logging::Logger *GetLogger() { return logger_; }

  PFN_vkVoidFunction getProcAddr(::VkInstance instance, const char *function);

private:
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;

  logging::Logger *logger_;
  std::unique_ptr<dynamic_loader::DynamicLibrary> vulkan_lib_;
};
}