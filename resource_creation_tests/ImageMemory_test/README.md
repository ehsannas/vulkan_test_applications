# vkGetImageMemoryRequirements

## Signatures
```c++
void vkGetImageMemoryRequirements(
    VkDevice                                    device,
    VkImage                                     image,
    VkMemoryRequirements*                       pMemoryRequirements);
```

# VkMemoryRequirements
```c++
typedef struct VkMemoryRequirements {
    VkDeviceSize    size;
    VkDeviceSize    alignment;
    uint32_t        memoryTypeBits;
} VkMemoryRequirements;
```

According to the Vulkan spec:
- `device` **must** be a valid device
- `image` **must** have come from device
- `VkMemoryRequirements` **must** be a pointer to a VkMemoryRequirements
structure

These tests should test the following cases:
- [x] Valid usage