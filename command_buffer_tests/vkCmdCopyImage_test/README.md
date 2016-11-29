# vkCmdCopyImage

## Signatures
```c++
void vkCmdCopyImage(
    VkCommandBuffer                             commandBuffer,
    VkImage                                     srcImage,
    VkImageLayout                               srcImageLayout,
    VkImage                                     dstImage,
    VkImageLayout                               dstImageLayout,
    uint32_t                                    regionCount,
    const VkImageCopy*                          pRegions);
```

# VkImageCopy
```c++
typedef struct VkImageCopy {
    VkImageSubresourceLayers    srcSubresource;
    VkOffset3D                  srcOffset;
    VkImageSubresourceLayers    dstSubresource;
    VkOffset3D                  dstOffset;
    VkExtent3D                  extent;
} VkImageCopy;
```

# VkImageSubresourceLayers
```c++
typedef struct VkImageSubresourceLayers {
    VkImageAspectFlags    aspectMask;
    uint32_t              mipLevel;
    uint32_t              baseArrayLayer;
    uint32_t              layerCount;
} VkImageSubresourceLayers;
```

According to the Vulkan spec:
- The format of `srcImage` and the `dstImage` **must** be compatible. Formats
  are considered compatible if their element size is the same between both
  formats.
- `srcImage` **must** have been created with `VK_IMAGE_USAGE_TRANSFER_SRC_BIT`
  usage flag
- `srcImageLayout` **must** be either of `VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL`
  or `VK_IMAGE_LAYOUT_GENERAL`
- `dstImage` **must** have been created with `VK_IMAGE_USAGE_TRANSFER_DST_BIT`
  usage flag
- `dstImageLayout` **must** be either of `VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`
  or `VK_IMAGE_LAYOUT_GENERAL`
- The sample count of `srcImage` and `dstImage` **must** match
- `pRegions` **must** be a pointer to an array of `regionCount` valid
  `VkImageCopy` structures
- `regionCount` **must** be greater than 0
- The union of all source regions, and the union of all destination regions,
  specified by the elements of `pRegions`, **must not** overlap in memory
- The `VkCommandPool` that `commandBuffer` was allocated from **must** support
  transfer, graphics, or compute operations.
- If `srcImage` is compressed, then:
  - If `extent.width` is not a multiple of the compressed texel block width,
  then `(extent.width + srcOffset.x)` must equal the image subresource width.
  - If `extent.height` is not a multiple of the compressed texel block height,
  then `(extent.height + srcOffset.y)` must equal the image subresource height.
  - If `extent.depth` is not a multiple of the compressed texel block depth,
  then `(extent.depth + srcOffset.z)` must equal the image subresource depth.
- If `dstImage` is compressed, then:
  - If `extent.width` is not a multiple of the compressed texel block width,
  then `(extent.width + dstOffset.x)` must equal the image subresource width.
  - If `extent.height` is not a multiple of the compressed texel block height,
  then `(extent.height + dstOffset.y)` must equal the image subresource height.
  - If `extent.depth` is not a multiple of the compressed texel block depth,
  then `(extent.depth + dstOffset.z)` must equal the image subresource depth.
- The `aspectMask` of member of `srcSubresource` and `dstSubresource` **must**
  match
- The `layerCount` member of `srcSubresource` and `dstSubresource` **must**
  match
- If either of the calling command’s `srcImage` or `dstImage` parameters are of
  `VkImageType VK_IMAGE_TYPE_3D`, the `baseArrayLayer` and `layerCount` members
  of both `srcSubresource` and `dstSubresource` must be 0 and 1, respectively
- The `aspectMask` member of `srcSubresource` **must** specify aspects present
  in the calling command's `srcImage`
- The `aspectMask` member of `dstSubresource` **must** specify aspects present
  in the calling command's `dstImage`

These tests should test the following cases:
- [x] `regionCount` of value 1
- [ ] `regionCount` of value more than 1
- [x] `srcOffset` of value 0
- [ ] `srcOffset` of value other than 0
- [x] `dstOffset` of value 0
- [ ] `dstOffset` of value other than 0
- [x] `mipLevel` of value 0
- [ ] `mipLevel` of value other than 0
- [x] `baseArrayLayer` of value 0
- [ ] `baseArrayLayer` of value other than 0
- [x] `layerCount` of value 1
- [ ] `layerCount` of value more than 1
