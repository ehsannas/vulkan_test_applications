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

include(CMakeParseArguments)

set(CONFIGURABLE_ANDROID_SOURCES
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/build.gradle
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/gradle.properties
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/local.properties
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/settings.gradle
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/build.gradle
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/CMakeLists.txt
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/proguard-rules.pro
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/src/main/AndroidManifest.xml
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/src/main/res/values/colors.xml
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/src/main/res/values/strings.xml
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/src/main/res/values/styles.xml)

set(NON_CONFIGURABLE_ANDROID_SOURCES
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/gradlew
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/gradlew.bat
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/gradle/wrapper/gradle-wrapper.jar
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/gradle/wrapper/gradle-wrapper.properties
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/src/main/res/mipmap-hdpi/ic_launcher.png
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/src/main/res/mipmap-mdpi/ic_launcher.png
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/src/main/res/mipmap-xhdpi/ic_launcher.png
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/src/main/res/mipmap-xxhdpi/ic_launcher.png
  ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template/app/src/main/res/mipmap-xxxhdpi/ic_launcher.png)

# This recursively gathers all of the dependencies for a target.
macro(gather_deps target)
  get_target_property(SRC ${target} LIB_SRCS)
  get_target_property(LIBS ${target} LIB_DEPS)
  if (SRC)
    list(APPEND SOURCE_DEPS ${SRC})
  endif()
  if (LIBS)
    foreach(LIB ${LIBS})
      gather_deps(${LIB})
    endforeach()
  endif()
endmacro()

# This adds a vulkan executable (program). By default this just plugs into
# add_executable (Linux/Windows) or add_library (Android). If BUILD_APKS
# is true, then an Android Studio project is created and a target to build it
# is created. All of the dependencies are correctly tracked and the
# project will be rebuilt if any dependency changes.
function(add_vulkan_executable target)
  cmake_parse_arguments(EXE "" "" "SOURCES;LIBS" ${ARGN})

  if (ANDROID)
    add_library(${target} SHARED ${EXE_SOURCES})
    if (EXE_LIBS)
      target_link_libraries(${target} PRIVATE ${EXE_LIBS})
    endif()
    target_link_libraries(${target} PRIVATE entry)
  elseif (NOT BUILD_APKS)
    add_executable(${target} ${EXE_SOURCES} ${EXE_UNPARSED_ARGS})
    if (EXE_LIBS)
      target_link_libraries(${target} PRIVATE ${EXE_LIBS})
    endif()
    target_link_libraries(${target} PRIVATE entry)
  else()
    set(TARGET_SOURCES)
    set(ANDROID_TARGET_NAME ${target})
    set(${target}_SOURCES ${EXE_SOURCES})
    set(${target}_LIBS ${EXE_LIBS})

    foreach(source ${CONFIGURABLE_ANDROID_SOURCES})
      file(RELATIVE_PATH rooted_source ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template ${source})
      configure_file(${source} ${CMAKE_CURRENT_BINARY_DIR}/${target}-apk/${rooted_source} @ONLY)
      list(APPEND TARGET_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${target}-apk/${rooted_source})
    endforeach()
    foreach(source ${NON_CONFIGURABLE_ANDROID_SOURCES})
      file(RELATIVE_PATH rooted_source ${VulkanTestApplications_SOURCE_DIR}/cmake/android_project_template ${source})
      configure_file(${source} ${CMAKE_CURRENT_BINARY_DIR}/${target}-apk/${rooted_source} COPYONLY)
      list(APPEND TARGET_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${target}-apk/${rooted_source})
    endforeach()

    set(apk_build_location "${CMAKE_CURRENT_BINARY_DIR}/${target}-apk/app/build/outputs/apk/app-debug.apk")
    set(target_apk ${CMAKE_CURRENT_BINARY_DIR}/${target}.apk)

    add_custom_target(${target}_sources)
    set(ABSOLUTE_SOURCES)
    foreach(SOURCE ${EXE_SOURCES})
      get_filename_component(TEMP ${SOURCE} ABSOLUTE)
      list(APPEND ABSOLUTE_SOURCES ${TEMP})
    endforeach()
    set_target_properties(${target}_sources PROPERTIES LIB_SRCS "${ABSOLUTE_SOURCES}")
    set_target_properties(${target}_sources PROPERTIES LIB_DEPS "${EXE_LIBS}")

    set(SOURCE_DEPS)
    gather_deps(${target}_sources)

    add_custom_command(
      OUTPUT ${target_apk}
      COMMAND ./gradlew build
      COMMAND ${CMAKE_COMMAND} -E copy ${apk_build_location} ${target_apk}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${target}-apk
      DEPENDS ${SOURCE_DEPS}
              ${TARGET_SOURCES})
    add_custom_target(${target} ALL DEPENDS ${target_apk})
  endif()
endfunction(add_vulkan_executable)


# This adds a library. By default this just plugs into
# add_library. If BUILD_APKS is true then a custom target is created
# and all of the sources are added to it. When an executable
# (from above) uses the library, the sources treated AS
# dependencies.
function(_add_vulkan_library target)
  cmake_parse_arguments(LIB "" "TYPE" "SOURCES;LIBS" ${ARGN})

  if (BUILD_APKS)
    add_custom_target(${target})
    set(ABSOLUTE_SOURCES)
    foreach(SOURCE ${LIB_SOURCES})
      get_filename_component(TEMP ${SOURCE} ABSOLUTE)
      list(APPEND ABSOLUTE_SOURCES ${TEMP})
    endforeach()
    set_target_properties(${target} PROPERTIES LIB_SRCS "${ABSOLUTE_SOURCES}")
    set_target_properties(${target} PROPERTIES LIB_DEPS "${LIB_LIBS}")
  else()
    add_library(${target} ${LIB_TYPE} ${LIB_SOURCES})
    target_link_libraries(${target} PRIVATE ${LIB_LIBS})
  endif()
endfunction()

# Helper function to add a static libray
function(add_vulkan_static_library target)
  _add_vulkan_library(${target} TYPE STATIC ${ARGN})
endfunction(add_vulkan_static_library)

# Helper function to add a shared libray
function(add_vulkan_shared_library target)
  _add_vulkan_library(${target} TYPE SHARED ${ARGN})
endfunction(add_vulkan_shared_library)
