# vkCmdDraw

## Signatures
```c++
void vkCmdDraw(
    VkCommandBuffer                             commandBuffer,
    uint32_t                                    vertexCount,
    uint32_t                                    instanceCount,
    uint32_t                                    firstVertex,
    uint32_t                                    firstInstance);
```

According to the Vulkan spec:
- A valid graphics pipeline **must** be bound to the current command buffer
  with `VK_PIPELINE_BIND_POINT_GRAPHICS`
- `commandBuffer` **must** be in the recording state
- This command **must** only be called inside of a render pass instance

These tests should test the following cases:
- [ ] `vertexCount` == 0
- [x] `vertexCount` > 0
- [ ] `instanceCount` == 0
- [x] `instanceCount` > 0
- [x] `firstVertex` == 0
- [ ] `firstVertex` > 0
- [x] `firstInstance` == 0
- [ ] `firstInstance` > 0
