// TextureImage.h
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <string>
#include "VulkanApp.h"

// Helper that loads a KTX texture, uploads it via a staging buffer and
// returns a filled `Texture` (defined in `VulkanApp.h`). This encapsulates
// the image, view, sampler and VMA allocation used for the image.
class TextureImage {
public:
    TextureImage() = default;
    ~TextureImage() = default;

    // Load a texture from `path`. On failure the returned Texture will have
    // `image == VK_NULL_HANDLE`.
    Texture load(VkDevice device, VmaAllocator allocator, VkCommandPool oneTimeCmdPool, VkQueue queue, const std::string& path) const;
};
