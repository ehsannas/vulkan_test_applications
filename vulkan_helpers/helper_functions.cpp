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

#include "vulkan_helpers/helper_functions.h"

#include <algorithm>

#include "support/containers/vector.h"
#include "support/log/log.h"

namespace vulkan {
VkInstance CreateEmptyInstance(containers::Allocator* allocator,
                               LibraryWrapper* wrapper) {
  // Test a non-nullptr pApplicationInfo
  VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             nullptr,
                             "TestApplication",
                             1,
                             "Engine",
                             0,
                             VK_MAKE_VERSION(1, 0, 0)};

  VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                            nullptr,
                            0,
                            &app_info,
                            0,
                            nullptr,
                            0,
                            nullptr};

  ::VkInstance raw_instance;
  LOG_ASSERT(==, wrapper->GetLogger(),
             wrapper->vkCreateInstance(&info, nullptr, &raw_instance),
             VK_SUCCESS);
  // vulkan::VkInstance will handle destroying the instance
  return vulkan::VkInstance(allocator, raw_instance, nullptr, wrapper);
}

VkInstance CreateDefaultInstance(containers::Allocator* allocator,
                                 LibraryWrapper* wrapper) {
  // Similar to Empty Instance, but turns on the platform specific
  // swapchian functions.
  // Test a non-nullptr pApplicationInfo
  VkApplicationInfo app_info{VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             nullptr,
                             "TestApplication",
                             1,
                             "Engine",
                             0,
                             VK_MAKE_VERSION(1, 0, 0)};

  const char* extensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#if defined __ANDROID__
    VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined __linux__
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#elif defined __WIN32__
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
  };

  wrapper->GetLogger()->LogInfo("Enabled Extensions: ");
  for (auto& extension : extensions) {
    wrapper->GetLogger()->LogInfo("    ", extension);
  }

  VkInstanceCreateInfo info{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                            nullptr,
                            0,
                            &app_info,
                            0,
                            nullptr,
                            sizeof(extensions) / sizeof(extensions[0]),
                            extensions};

  ::VkInstance raw_instance;
  LOG_ASSERT(==, wrapper->GetLogger(),
             wrapper->vkCreateInstance(&info, nullptr, &raw_instance),
             VK_SUCCESS);
  // vulkan::VkInstance will handle destroying the instance
  return vulkan::VkInstance(allocator, raw_instance, nullptr, wrapper);
}

containers::vector<VkPhysicalDevice> GetPhysicalDevices(
    containers::Allocator* allocator, VkInstance& instance) {
  uint32_t device_count = 0;
  instance->vkEnumeratePhysicalDevices(instance, &device_count, nullptr);

  containers::vector<VkPhysicalDevice> physical_devices(device_count,
                                                        allocator);
  LOG_ASSERT(==, instance.GetLogger(),
             instance->vkEnumeratePhysicalDevices(instance, &device_count,
                                                  physical_devices.data()),
             VK_SUCCESS);

  return std::move(physical_devices);
}

containers::vector<VkQueueFamilyProperties> GetQueueFamilyProperties(
    containers::Allocator* allocator, VkInstance& instance,
    ::VkPhysicalDevice device) {
  uint32_t count = 0;
  instance->vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);

  LOG_ASSERT(>, instance.GetLogger(), count, 0u);
  containers::vector<VkQueueFamilyProperties> properties(allocator);
  properties.resize(count);
  instance->vkGetPhysicalDeviceQueueFamilyProperties(device, &count,
                                                     properties.data());
  return std::move(properties);
}

inline bool HasGraphicsAndComputeQueue(
    const VkQueueFamilyProperties& property) {
  return property.queueCount > 0 &&
         ((property.queueFlags &
           (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) ==
          (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT));
}

uint32_t GetGraphicsAndComputeQueueFamily(containers::Allocator* allocator,
                                          VkInstance& instance,
                                          ::VkPhysicalDevice device) {
  auto properties = GetQueueFamilyProperties(allocator, instance, device);

  for (uint32_t i = 0; i < properties.size(); ++i) {
    if (HasGraphicsAndComputeQueue(properties[i])) return i;
  }
  return ~0u;
}

VkDevice CreateDefaultDevice(containers::Allocator* allocator,
                             VkInstance& instance,
                             bool require_graphics_compute_queue) {
  containers::vector<VkPhysicalDevice> physical_devices =
      GetPhysicalDevices(allocator, instance);
  float priority = 1.f;

  VkPhysicalDevice physical_device = physical_devices.front();

  VkPhysicalDeviceProperties properties;
  instance->vkGetPhysicalDeviceProperties(physical_device, &properties);

  const uint32_t queue_family_index =
      require_graphics_compute_queue ? GetGraphicsAndComputeQueueFamily(
                                           allocator, instance, physical_device)
                                     : 0;
  LOG_ASSERT(!=, instance.GetLogger(), queue_family_index, ~0u);

  VkDeviceQueueCreateInfo queue_info{
      /* sType = */ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      /* pNext = */ nullptr,
      /* flags = */ 0,
      /* queueFamilyIndex = */ queue_family_index,
      /* queueCount */ 1,
      /* pQueuePriorities = */ &priority};

  VkDeviceCreateInfo info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                          nullptr,
                          0,
                          1,
                          &queue_info,
                          0,
                          nullptr,
                          0,
                          nullptr,
                          nullptr};

  ::VkDevice raw_device;
  LOG_ASSERT(
      ==, instance.GetLogger(),
      instance->vkCreateDevice(physical_device, &info, nullptr, &raw_device),
      VK_SUCCESS);
  return vulkan::VkDevice(allocator, raw_device, nullptr, &instance,
                          &properties, physical_device);
}

VkDevice CreateDeviceForSwapchain(containers::Allocator* allocator,
                                  VkInstance* instance, VkSurfaceKHR* surface,
                                  uint32_t* present_queue_index,
                                  uint32_t* graphics_queue_index) {
  containers::vector<VkPhysicalDevice> physical_devices =
      GetPhysicalDevices(allocator, *instance);
  float priority = 1.f;

  for (auto device : physical_devices) {
    VkPhysicalDevice physical_device = device;
    VkPhysicalDeviceProperties physical_device_properties;
    (*instance)->vkGetPhysicalDeviceProperties(device,
                                               &physical_device_properties);

    auto properties = GetQueueFamilyProperties(allocator, *instance, device);
    const uint32_t graphics_queue_family_index =
        GetGraphicsAndComputeQueueFamily(allocator, *instance, physical_device);
    uint32_t present_queue_family_index = 0;
    for (; present_queue_family_index < properties.size();
         ++present_queue_family_index) {
      VkBool32 supports_swapchain = false;
      LOG_EXPECT(==, instance->GetLogger(),
                 (*instance)->vkGetPhysicalDeviceSurfaceSupportKHR(
                     device, present_queue_family_index, *surface,
                     &supports_swapchain),
                 VK_SUCCESS);
      if (supports_swapchain) {
        break;
      }
    }

    if (present_queue_family_index == properties.size()) {
      continue;
    }

    VkDeviceQueueCreateInfo queue_infos[] = {
        {/* sType = */ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         /* pNext = */ nullptr,
         /* flags = */ 0,
         /* queueFamilyIndex = */ graphics_queue_family_index,
         /* queueCount */ 1,
         /* pQueuePriorities = */ &priority},
        {/* sType = */ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         /* pNext = */ nullptr,
         /* flags = */ 0,
         /* queueFamilyIndex = */ present_queue_family_index,
         /* queueCount */ 1,
         /* pQueuePriorities = */ &priority},
    };

    const char* enabled_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo info{
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,  // stype
        nullptr,                               // pNext
        0,                                     // flags
        (present_queue_family_index !=
         ((unsigned int)graphics_queue_family_index))
            ? 2u
            : 1u,            // queueCreateInfoCount
        queue_infos,         // pQueueCreateInfos
        0,                   // enabledLayerCount
        nullptr,             // ppEnabledLayerNames
        1,                   // enabledExtensionCount
        enabled_extensions,  // ppEnabledExtensionNames
        nullptr              // ppEnabledFeatures
    };

    ::VkDevice raw_device;
    LOG_ASSERT(==, instance->GetLogger(),
               (*instance)->vkCreateDevice(physical_device, &info, nullptr,
                                           &raw_device),
               VK_SUCCESS);
    *present_queue_index = present_queue_family_index;
    *graphics_queue_index = graphics_queue_family_index;
    return vulkan::VkDevice(allocator, raw_device, nullptr, instance,
                            &physical_device_properties, physical_device);
  }
  LOG_CRASH(instance->GetLogger(),
            "Could not find physical device or queue that can present");
}

VkCommandPool CreateDefaultCommandPool(containers::Allocator* allocator,
                                       VkDevice& device) {
  VkCommandPoolCreateInfo info = {
      /* sType = */ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      /* pNext = */ nullptr,
      /* flags = */ VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
      // TODO(antiagainst): use a graphics + compute queue family.
      /* queueFamilyIndex = */ 0,
  };

  ::VkCommandPool raw_command_pool;
  LOG_ASSERT(
      ==, device.GetLogger(),
      device->vkCreateCommandPool(device, &info, nullptr, &raw_command_pool),
      VK_SUCCESS);
  return vulkan::VkCommandPool(raw_command_pool, nullptr, &device);
}

VkSurfaceKHR CreateDefaultSurface(VkInstance* instance,
                                  const entry::entry_data* data) {
  ::VkSurfaceKHR surface;
#if defined __ANDROID__
  VkAndroidSurfaceCreateInfoKHR create_info{
      VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR, 0, 0,
      data->native_window_handle};

  (*instance)->vkCreateAndroidSurfaceKHR(*instance, &create_info, nullptr,
                                         &surface);
#elif defined __linux__
  VkXcbSurfaceCreateInfoKHR create_info{
      VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR, 0, 0,
      data->native_connection, data->native_window_handle};

  (*instance)->vkCreateXcbSurfaceKHR(*instance, &create_info, nullptr,
                                     &surface);
#elif defined __WIN32__
  VkWin32SurfaceCreateInfo create_info{
      VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, 0, 0,
      data->native_hinstance, data->nativE_window_handle};

  (*instance)->vkCreateWin32SurfaceKHR(*instance, &create_info, nullptr,
                                       &surface);
#endif

  return VkSurfaceKHR(surface, nullptr, instance);
}

VkCommandBuffer CreateDefaultCommandBuffer(VkCommandPool* pool,
                                           VkDevice* device) {
  VkCommandBufferAllocateInfo info = {
      /* sType = */ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      /* pNext = */ nullptr,
      /* commandPool = */ pool->get_raw_object(),
      /* level = */ VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      /* commandBufferCount = */ 1,
  };
  ::VkCommandBuffer raw_command_buffer;
  LOG_ASSERT(==, device->GetLogger(), (*device)->vkAllocateCommandBuffers(
                                          *device, &info, &raw_command_buffer),
             VK_SUCCESS);
  return vulkan::VkCommandBuffer(raw_command_buffer, pool, device);
}

VkSwapchainKHR CreateDefaultSwapchain(VkInstance* instance, VkDevice* device,
                                      VkSurfaceKHR* surface,
                                      containers::Allocator* allocator,
                                      uint32_t present_queue_index,
                                      uint32_t graphics_queue_index) {
  const bool has_multiple_queues = present_queue_index != graphics_queue_index;
  const uint32_t queues[2] = {graphics_queue_index, present_queue_index};
  VkSurfaceCapabilitiesKHR surface_caps;
  LOG_ASSERT(==, instance->GetLogger(),
             (*instance)->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                 device->physical_device(), *surface, &surface_caps),
             VK_SUCCESS);

  uint32_t num_formats = 0;
  LOG_ASSERT(==, instance->GetLogger(),
             (*instance)->vkGetPhysicalDeviceSurfaceFormatsKHR(
                 device->physical_device(), *surface, &num_formats, nullptr),
             VK_SUCCESS);

  containers::vector<VkSurfaceFormatKHR> surface_formats(num_formats,
                                                         allocator);
  LOG_ASSERT(==, instance->GetLogger(),
             (*instance)->vkGetPhysicalDeviceSurfaceFormatsKHR(
                 device->physical_device(), *surface, &num_formats,
                 surface_formats.data()),
             VK_SUCCESS);

  uint32_t num_present_modes = 0;
  LOG_ASSERT(
      ==, instance->GetLogger(),
      (*instance)->vkGetPhysicalDeviceSurfacePresentModesKHR(
          device->physical_device(), *surface, &num_present_modes, nullptr),
      VK_SUCCESS);
  containers::vector<VkPresentModeKHR> present_modes(num_present_modes,
                                                     allocator);
  LOG_ASSERT(==, instance->GetLogger(),
             (*instance)->vkGetPhysicalDeviceSurfacePresentModesKHR(
                 device->physical_device(), *surface, &num_present_modes,
                 present_modes.data()),
             VK_SUCCESS);

  uint32_t chosenAlpha =
      static_cast<uint32_t>(surface_caps.supportedCompositeAlpha);
  LOG_ASSERT(!=, instance->GetLogger(), 0,
             surface_caps.supportedCompositeAlpha);

  // Time to get the LSB of chosenAlpha;

  chosenAlpha = GetLSB(chosenAlpha);

  VkExtent2D image_extent(surface_caps.currentExtent);
  if (image_extent.width == 0xFFFFFFFF) {
    image_extent = VkExtent2D{100, 100};
  }

  VkSwapchainCreateInfoKHR swapchainCreateInfo{
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,  // sType
      nullptr,                                      // pNext
      0,                                            // flags
      *surface,                                     // surface
      std::min(surface_caps.minImageCount + 1,
               surface_caps.maxImageCount),  // minImageCount
      surface_formats[0].format,             // surfaceFormat
      surface_formats[0].colorSpace,         // colorSpace
      image_extent,                          // imageExtent
      1,                                     // imageArrayLayers
      VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,   // imageUsage
      has_multiple_queues ? VK_SHARING_MODE_CONCURRENT
                          : VK_SHARING_MODE_EXCLUSIVE,  // sharingMode
      has_multiple_queues ? 2u : 0u,
      has_multiple_queues ? queues : nullptr,  // pQueueFamilyIndices
      surface_caps.currentTransform,           // preTransform,
      static_cast<VkCompositeAlphaFlagBitsKHR>(chosenAlpha),  // compositeAlpha
      present_modes.front(),                                  // presentModes
      false,                                                  // clipped
      VK_NULL_HANDLE                                          // oldSwapchain
  };

  ::VkSwapchainKHR swapchain;

  LOG_ASSERT(==, instance->GetLogger(),
             (*device)->vkCreateSwapchainKHR(*device, &swapchainCreateInfo,
                                             nullptr, &swapchain),
             VK_SUCCESS);
  return VkSwapchainKHR(swapchain, nullptr, device, image_extent.width,
                        image_extent.height, 1u, surface_formats[0].format);
}

VkImage CreateDefault2DColorImage(VkDevice* device, uint32_t width,
                                  uint32_t height) {
  VkImageCreateInfo info{
      /* sType = */ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
      /* pNext = */ nullptr,
      /* flags = */ 0,
      /* imageType = */ VK_IMAGE_TYPE_2D,
      /* format = */ VK_FORMAT_R8G8B8A8_UNORM,
      /* extent = */ {
          /* width = */ width,
          /* height = */ height,
          /* depth = */ 1,
      },
      /* mipLevels = */ 1,
      /* arrayLayers = */ 1,
      /* samples = */ VK_SAMPLE_COUNT_1_BIT,
      /* tiling = */ VK_IMAGE_TILING_OPTIMAL,
      /* usage = */ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      /* sharingMode = */ VK_SHARING_MODE_EXCLUSIVE,
      /* queueFamilyIndexCount = */ 0,
      /* pQueueFamilyIndices = */ nullptr,
      /* initialLayout = */ VK_IMAGE_LAYOUT_UNDEFINED,
  };
  ::VkImage raw_image;
  LOG_ASSERT(==, device->GetLogger(),
             (*device)->vkCreateImage(*device, &info, nullptr, &raw_image),
             VK_SUCCESS);
  return vulkan::VkImage(raw_image, nullptr, device);
}
}
