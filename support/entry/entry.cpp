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

#include "support/entry/entry.h"

#include <cassert>
#include <chrono>
#include <thread>

#include "support/log/log.h"

namespace entry {
namespace internal {
void dummy_function() {}
}
}

#if defined __ANDROID__
#include <android_native_app_glue.h>
#include <unistd.h>

// This method is called by android_native_app_glue. This is the main entry
// point for any native android activity.
void android_main(android_app* app) {
  // Simply wait for 10 seconds, this is useful if we have to attach late.
  if (access("/sdcard/wait-for-debugger.txt", F_OK) != -1) {
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  // Hack to make sure android_native_app_glue is not stripped.
  app_dummy();
  containers::Allocator root_allocator;

  std::thread main_thread([&]() {
    entry::entry_data data{app, logging::GetLogger(&root_allocator),
                           &root_allocator};
    int return_value = main_entry(&data);
    // Do not modify this line, scripts may look for it in the output.
    data.log->LogInfo("RETURN: ", return_value);
  });

  while (1) {
    // Read all pending events.
    int ident = 0;
    int events = 0;
    struct android_poll_source* source = nullptr;
    while ((ident = ALooper_pollAll(-1, NULL, &events, (void**)&source)) >= 0) {
      if (source) {
        source->process(app, source);
      }
      if (app->destroyRequested) {
        break;
      }
    }
    if (app->destroyRequested) {
      break;
    }
  }
  main_thread.join();
  assert(root_allocator.currently_allocated_bytes_.load() == 0);
}
#elif defined __linux__

// This creates an XCB connection and window for the application.
// It maps it onto the screen and passes it on to the main_entry function.
int main(int argc, char** argv) {
  containers::Allocator root_allocator;
  xcb_connection_t* connection = xcb_connect(NULL, NULL);
  const xcb_setup_t* setup = xcb_get_setup(connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  xcb_screen_t* screen = iter.data;

  xcb_window_t window = xcb_generate_id(connection);
  xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0,
                    0, 100, 100, 1, XCB_WINDOW_CLASS_INPUT_OUTPUT,
                    screen->root_visual, 0, NULL);

  xcb_map_window(connection, window);
  xcb_flush(connection);

  int return_value = 0;

  std::thread main_thread([&]() {
    entry::entry_data data{window, connection,
                           logging::GetLogger(&root_allocator),
                           &root_allocator};
    return_value = main_entry(&data);
  });
  main_thread.join();
  // TODO(awoloszyn): Handle other events here.
  xcb_disconnect(connection);
  assert(root_allocator.currently_allocated_bytes_.load() == 0);
  return return_value;
}
#elif defined _WIN32
#error TODO(awoloszyn): Handle the win32 entry point.
#error This means setting up a Win32 window, and passing it through the entry_data.
#else
#error Unsupported platform.
#endif
