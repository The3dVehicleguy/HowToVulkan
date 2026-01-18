// Asset.cpp
#include <volk/volk.h>
#include "Asset.h"
#include <iostream>

bool Asset::Load(VkDevice device, VmaAllocator allocator, VkCommandPool cmdPool, VkQueue queue, const std::string& modelFile,
                 const std::vector<std::string>& textureFiles, const VmaAllocationCreateInfo& modelAllocInfo)
{
    device_ = device;
    // Load model
    if (!model_.LoadFromObj(modelFile, allocator, device, modelAllocInfo)) 
    {
        return false;
    }

    // Load textures
    textures_.clear();
    std::vector<VkDescriptorImageInfo> imageInfos;
    for (auto& tf : textureFiles) 
    {
        textures_.emplace_back(device, allocator, cmdPool, queue, tf);
        const Texture& t = textures_.back().get();
        if (t.image == VK_NULL_HANDLE) return false;
        imageInfos.push_back({ .sampler = t.sampler, .imageView = t.view, .imageLayout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL });
    }

    // Create descriptor set for these textures
    descriptorOwned_ = DescriptorRAII(device, imageInfos);
    return true;
}

void Asset::Destroy(VkDevice device, VmaAllocator allocator)
{
    // Destroy textures
    for (auto& th : textures_) th.destroy(device, allocator);
    textures_.clear();
    // Destroy descriptor resources
    descriptorOwned_.destroy(device);
    // Destroy model buffer
    model_.Destroy(device);
}
