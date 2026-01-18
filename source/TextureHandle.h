#pragma once
#include "VulkanApp.h" // for Texture struct
#include "TextureImage.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <string>
#include <iostream>

// RAII wrapper around loaded Texture. Provides explicit destroy(device, allocator)
// to ensure correct teardown order relative to allocator/device.
class TextureHandle {
public:
    TextureHandle() = default;
    TextureHandle(VkDevice device, VmaAllocator allocator, VkCommandPool cmdPool, VkQueue queue, const std::string& filename)
    {
        TextureImage loader;
        tex_ = loader.load(device, allocator, cmdPool, queue, filename);
        if (tex_.image == VK_NULL_HANDLE) {
            std::cerr << "TextureHandle: failed to load " << filename << '\n';
        }
    }

    ~TextureHandle()
    {
        if (!destroyed_) {
            // best-effort: user should call destroy explicitly before allocator/device teardown
        }
    }

    void destroy(VkDevice device, VmaAllocator allocator)
    {
        if (!destroyed_) {
            if (tex_.view != VK_NULL_HANDLE) vkDestroyImageView(device, tex_.view, nullptr);
            if (tex_.sampler != VK_NULL_HANDLE) vkDestroySampler(device, tex_.sampler, nullptr);
            if (tex_.image != VK_NULL_HANDLE) vmaDestroyImage(allocator, tex_.image, tex_.allocation);
            destroyed_ = true;
        }
    }

    const Texture& get() const { return tex_; }

private:
    Texture tex_{};
    bool destroyed_{ false };
};
