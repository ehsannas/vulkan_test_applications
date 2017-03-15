# Copyright 2016 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

if(ANDROID)
set(ADDITIONAL_LIBS android android_native_app_glue atomic)
elseif(UNIX)
set(ADDITIONAL_LIBS xcb pthread)
endif()


if (NOT DEFAULT_WINDOW_WIDTH)
  set(DEFAULT_WINDOW_WIDTH 100)
endif()

if (NOT DEFAULT_WINDOW_HEIGHT)
  set(DEFAULT_WINDOW_HEIGHT 100)
endif()

SET(DEFAULT_WINDOW_WIDTH ${DEFAULT_WINDOW_WIDTH} CACHE INT
    "Default window width for platforms that have resizable windows")
SET(DEFAULT_WINDOW_HEIGHT ${DEFAULT_WINDOW_HEIGHT} CACHE INT
    "Default window height for platforms that have resizable windows")

add_vulkan_static_library(entry
    SOURCES
        entry.cpp
    LIBS
        ${ADDITIONAL_LIBS}
        logger
        containers)

if (NOT BUILD_APKS)
    target_compile_options(entry PRIVATE
        -DDEFAULT_WINDOW_WIDTH=${DEFAULT_WINDOW_WIDTH}
        -DDEFAULT_WINDOW_HEIGHT=${DEFAULT_WINDOW_HEIGHT})
endif()
