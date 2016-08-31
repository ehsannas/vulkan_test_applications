# Physical Device Image Format Traits

## Signatures
```c++
VkResult vkGetPhysicalDeviceImageFormatProperties(
    VkPhysicalDevice                            physicalDevice,
    VkFormat                                    format,
    VkImageType                                 type,
    VkImageTiling                               tiling,
    VkImageUsageFlags                           usage,
    VkImageCreateFlags                          flags,
    VkImageFormatProperties*                    pImageFormatProperties);
```

According to the Vulkan spec:
- `usage` **must not** be 0

## VkQueueFamilyProperties
```c++
typedef struct VkImageFormatProperties {
    VkExtent3D            maxExtent;
    uint32_t              maxMipLevels;
    uint32_t              maxArrayLayers;
    VkSampleCountFlags    sampleCounts;
    VkDeviceSize          maxResourceSize;
} VkImageFormatProperties;
```

These tests should test the following cases:
- [x] All combinations of
  - all valid `VkFormat` values
  - all valid `VkImageType` values
  - all valid `VkImageTiling` values
  - all valid `VkImageUsageFlags` values excluding 0
  - all valid `VkImageCreateFlags` values
