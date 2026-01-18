// Descriptor.h
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

// forward
struct VkDescriptorImageInfo;

// Minimal descriptor helper for creating a texture descriptor set layout.
class Descriptor 
{
public:
    Descriptor() = default;
    ~Descriptor();
    VkDescriptorSetLayout CreateLayout(VkDevice device, uint32_t bindingCount) const;
    VkDescriptorPool CreatePool(VkDevice device, uint32_t descriptorCount) const;
    VkDescriptorSet AllocateAndWrite(
        VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout,
        const std::vector<VkDescriptorImageInfo>& imageInfos) const;

    // Non-copyable
    Descriptor(const Descriptor&) = delete;
    Descriptor& operator=(const Descriptor&) = delete;

    // Movable
    Descriptor(Descriptor&&) noexcept;
    Descriptor& operator=(Descriptor&&) noexcept;

    // Initialize the descriptor helper: create a layout for bindingCount
    // combined image samplers and a pool sized for descriptorCount.
    // Returns true on success.
    bool Init(VkDevice device, uint32_t bindingCount = 1, uint32_t descriptorCount = 1);

    // Allocate a descriptor set from the internally owned pool/layout and
    // update it with imageInfos. Returns VK_NULL_HANDLE on failure.
    VkDescriptorSet AllocateAndWrite(const std::vector<VkDescriptorImageInfo>& imageInfos) const;

    // Accessors
    VkDescriptorPool GetPool() const { return pool_; }
    VkDescriptorSetLayout GetLayout() const { return layout_; }

private:
    VkDevice device_{ VK_NULL_HANDLE };
    VkDescriptorPool pool_{ VK_NULL_HANDLE };
    VkDescriptorSetLayout layout_{ VK_NULL_HANDLE };
};

