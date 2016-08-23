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

project(VulkanTestApplications)
cmake_minimum_required(VERSION 3.1)

# BUILD_APKS invokes all of the build in a special mode.
# Instead of actually creating libraries and executables, we generate Android Studio
# projects that depend on all of the libraries and executables. When we go and actually
# run ninja, we invoke gradle on any out of date projects.
# This means that there are 3 main modes that this build may be run in.
# 1) A normal cmake build. This builds targets for the host platform.
# 2a) A build with BUILD_APKS which will not itself create any libraries or executables, but
#      will create Android Studio projects to do the same. A build will
#      then run gradle on those projects which will run 2b) below.
# 2b) A build FROM Android Studio, this will set(ANDROID TRUE),
#      and generate shared libraries for use in an APK.

option(BUILD_APKS "Should we be building Android apks for our project"
  ${BUILD_APKS})

if (BUILD_APKS)
  set(ANDROID_SDK "" CACHE STRING "Location of the Android SDK (Sdk must contain gradle support)")
  set(ANDROID_NDK ${ANDROID_SDK}/ndk-bundle CACHE STRING "Location of the Android NDK (required r12b or higher)")
  get_filename_component(ANDROID_SDK ${ANDROID_SDK} ABSOLUTE)
  get_filename_component(ANDROID_NDK ${ANDROID_NDK} ABSOLUTE)
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include("cmake/build_apk.cmake")
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(support)

# Used for testing features, not actually code.
add_subdirectory(test_lib)
add_subdirectory(test_so)
add_subdirectory(test_apk)
