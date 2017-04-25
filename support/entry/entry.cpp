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
#include <cstdint>
#include <cstring>
#include <mutex>
#include <thread>
#include "entry_config.h"

#include "support/log/log.h"

namespace entry {
namespace internal {
void dummy_function() {}
}  // namespace internal
}  // namespace entry

#if defined __ANDROID__
#include <android_native_app_glue.h>
#include <unistd.h>

struct AppData {
  // Poor-man's semaphore, c++11 is missing a semaphore.
  std::mutex start_mutex;
};

// This is a  handler for all android app commands.
// Only handles APP_CMD_INIT_WINDOW right now, which unblocks
// the main thread, which needs the main window to be open
// before it does any WSI integration.
void HandleAppCommand(android_app* app, int32_t cmd) {
  AppData* data = (AppData*)app->userData;
  switch (cmd) {
    case APP_CMD_INIT_WINDOW:
      if (app->window != NULL) {
        // Wake the thread that is ready to go.
        data->start_mutex.unlock();
      }
      break;
  }
};

// This method is called by android_native_app_glue. This is the main entry
// point for any native android activity.
void android_main(android_app* app) {
  // Simply wait for 10 seconds, this is useful if we have to attach late.
  if (access("/sdcard/wait-for-debugger.txt", F_OK) != -1) {
    std::this_thread::sleep_for(std::chrono::seconds(10));
  }

  // Hack to make sure android_native_app_glue is not stripped.
  app_dummy();
  AppData data;
  data.start_mutex.lock();

  std::thread main_thread([&]() {
    data.start_mutex.lock();
    containers::Allocator root_allocator;
    {
      entry::entry_data data{app->window,
                             logging::GetLogger(&root_allocator),
                             &root_allocator,
                             {FIXED_TIMESTEP, PREFER_SEPARATE_PRESENT}};
      int return_value = main_entry(&data);
      // Do not modify this line, scripts may look for it in the output.
      data.log->LogInfo("RETURN: ", return_value);
    }
    assert(root_allocator.currently_allocated_bytes_.load() == 0);
  });

  app->userData = &data;
  app->onAppCmd = &HandleAppCommand;

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
}
#elif defined __linux__

// This creates an XCB connection and window for the application.
// It maps it onto the screen and passes it on to the main_entry function.
// -w=X will set the window width to X
// -h=Y will set the window height to Y
int main(int argc, char** argv) {
  uint32_t window_width = DEFAULT_WINDOW_WIDTH;
  uint32_t window_height = DEFAULT_WINDOW_HEIGHT;
  bool fixed_timestep = FIXED_TIMESTEP;
  bool prefer_separate_present = PREFER_SEPARATE_PRESENT;

  for (size_t i = 0; i < argc; ++i) {
    if (strncmp(argv[i], "-w=", 3) == 0) {
      window_width = atoi(argv[i] + 3);
    }
    if (strncmp(argv[i], "-h=", 3) == 0) {
      window_height = atoi(argv[i] + 3);
    }
    if (strncmp(argv[i], "-fixed", 6) == 0) {
      fixed_timestep = true;
    }
    if (strncmp(argv[i], "-separate-present", 17) == 0) {
      prefer_separate_present = true;
    }
  }

  containers::Allocator root_allocator;
  xcb_connection_t* connection = xcb_connect(NULL, NULL);
  const xcb_setup_t* setup = xcb_get_setup(connection);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
  xcb_screen_t* screen = iter.data;

  xcb_window_t window = xcb_generate_id(connection);
  xcb_create_window(connection, XCB_COPY_FROM_PARENT, window, screen->root, 0,
                    0, window_width, window_height, 1,
                    XCB_WINDOW_CLASS_INPUT_OUTPUT, screen->root_visual, 0,
                    NULL);

  xcb_map_window(connection, window);
  xcb_flush(connection);

  int return_value = 0;

  std::thread main_thread([&]() {
    entry::entry_data data{window,
                           connection,
                           logging::GetLogger(&root_allocator),
                           &root_allocator,
                           {fixed_timestep, prefer_separate_present}};
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
