project(VkCallbackSwapchain)
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT VULKAN_INCLUDE_LOCATION)
    set(VULKAN_INCLUDE_LOCATION "")
endif()

set(VULKAN_INCLUDE_LOCATION ${VULKAN_INCLUDE_LOCATION}
    CACHE STRING "Location of the Vulkan headers")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

add_library(VkLayer_CallbackSwapchain
  SHARED
    layer.h
    layer.cpp
    swapchain.h
    swapchain.cpp
    threading.h
    callback_swapchain.h
    callback_swapchain.cpp)

if(NOT "${VULKAN_INCLUDE_LOCATION}" STREQUAL "")
    target_include_directories(VkLayer_CallbackSwapchain
        PRIVATE
        ${VULKAN_INCLUDE_LOCATION})
endif()

set(SWAPCHAIN_LIBRARY_NAME "${CMAKE_SHARED_LIBRARY_PREFIX}VkLayer_CallbackSwapchain${CMAKE_SHARED_LIBRARY_SUFFIX}")
set(output_file CallbackSwapchainLayer.json)
if (CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(output_file ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/CallbackSwapchainLayer.json)
endif()
configure_file(CallbackSwapchainLayer.json
    ${output_file})