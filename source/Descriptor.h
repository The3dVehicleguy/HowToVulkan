// Descriptor.h
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

// forward
struct VkDescriptorImageInfo;

// Minimal descriptor helper for creating a texture descriptor set layout.
class Descriptor {
public:
    Descriptor() = default;
    ~Descriptor() = default;

    // Create a descriptor set layout suitable for an array of combined image samplers.
    // Returns VK_NULL_HANDLE on failure.
    VkDescriptorSetLayout createLayout(VkDevice device, uint32_t bindingCount = 1) const;

    // Create a simple descriptor pool able to allocate a single descriptor set containing
    // `descriptorCount` combined image samplers. Returns VK_NULL_HANDLE on failure.
    VkDescriptorPool createPool(VkDevice device, uint32_t descriptorCount = 1) const;

    // Allocate a descriptor set from `pool` with layout `layout` and update it with
    // the provided image infos. Returns VK_NULL_HANDLE on failure.
    VkDescriptorSet allocateAndWrite(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout, const std::vector<VkDescriptorImageInfo>& imageInfos) const;
};
