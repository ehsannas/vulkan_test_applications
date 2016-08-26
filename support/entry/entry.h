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

#ifndef SUPPORT_ENTRY_ENTRY_H_
#define SUPPORT_ENTRY_ENTRY_H_

#include <functional>
#include <memory>

#include "support/log/log.h"

#if defined __linux__ || defined __ANDROID__

#if defined __ANDROID__

struct android_app;

#else

#include <xcb/xcb.h>

#endif

namespace entry {
namespace internal {
// Hack to make sure that this entry-point library gets linked in properly.
void dummy_function();

struct dummy {
  dummy() { dummy_function(); }
};
} // namespace internal

static internal::dummy __attribute__((used)) test_dummy;
#elif defined _WIN32
typedef void *HANDLE;
#else
#error "Unsupported platform"
#endif

struct entry_data {
#if defined __ANDROID__
  android_app *native_window_handle;
#elif defined _WIN32
  HANDLE native_window_handle;
#elif defined __linux__
  xcb_window_t native_window_handle;
  xcb_connection_t *native_connection;
#endif
  // This is never null.
  std::unique_ptr<logging::Logger> log;
};
} // namespace entry

// This is the entry-point that every application should define.
int main_entry(const entry::entry_data *data);

#endif // SUPPORT_ENTRY_ENTRY_H_