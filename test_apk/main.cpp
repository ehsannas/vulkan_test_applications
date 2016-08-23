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

#include "support/dynamic_loader/dynamic_library.h"
#include "support/entry/entry.h"
#include "support/log/log.h"

// Trival entry function.
// It will be removed in the future once we have more function
// apps to test.
// It makes sure that both the logging, and the dynamic loader are functioning.
int main_entry(const entry::entry_data *data) {
  data->log->LogInfo("Application Startup");

  auto vulkan_lib = dynamic_loader::OpenLibrary("libvulkan");

  data->log->LogInfo("The Vulkan Library is ",
                      vulkan_lib && vulkan_lib->is_valid() ? "Valid"
                                                           : "Invalid");
  return 0;
}