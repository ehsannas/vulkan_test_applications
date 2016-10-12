# vkQueueSubmit

## Signatures
```c++
void vkCmdBindVertexBuffers(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    firstBinding,
    uint32_t                                    bindingCount,
    const VkBuffer*                             pBuffers,
    const VkDeviceSize*                         pOffsets);
```

According to the Vulkan spec:
- `pBuffers` **must** point to `bindingCount` `VkBuffer` structures
- `pOffsets` **must** point to `bindingCount` `VkDeviceSize` values
- `bindingCount` **must** be > 0
- `firstBinding` **must** be < `VkPhysicalDeviceLimits::maxVertexInputBindings`
- `firstBinding` + `bindingCount` **must** be <=
    `VkPhysicalDeviceLimits::maxVertexInputBindings`

These tests should test the following cases:
- [x] `bindingCount` == 1
- [ ] `bindingCount` > 1
- [x] `firstBinding` == 0
- [ ] `firstBinding` > 0
