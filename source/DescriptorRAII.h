#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "Descriptor.h"
#include <iostream>

// RAII wrapper for descriptor layout + pool + set. Uses the existing Descriptor
// helper to create resources and exposes an explicit destroy(device) method.
class DescriptorRAII {
public:
    DescriptorRAII() = default;
    DescriptorRAII(VkDevice device, const std::vector<VkDescriptorImageInfo>& textureDescriptors)
    {
        Descriptor helper;
        device_ = device;
        layout_ = helper.CreateLayout(device, static_cast<uint32_t>(textureDescriptors.size()));
        pool_ = helper.CreatePool(device, static_cast<uint32_t>(textureDescriptors.size()));
        if (pool_ == VK_NULL_HANDLE || layout_ == VK_NULL_HANDLE) {
            std::cerr << "DescriptorRAII: failed to create pool or layout\n";
            return;
        }
        set_ = helper.AllocateAndWrite(device, pool_, layout_, textureDescriptors);
        if (set_ == VK_NULL_HANDLE) {
            std::cerr << "DescriptorRAII: failed to allocate descriptor set\n";
        }
    }

    ~DescriptorRAII()
    {
        if (!destroyed_) {
            // best-effort; explicit destroy(device) preferred
        }
    }

    void destroy(VkDevice device)
    {
        if (!destroyed_) {
            if (pool_ != VK_NULL_HANDLE) {
                vkDestroyDescriptorPool(device, pool_, nullptr);
                pool_ = VK_NULL_HANDLE;
            }
            if (layout_ != VK_NULL_HANDLE) {
                vkDestroyDescriptorSetLayout(device, layout_, nullptr);
                layout_ = VK_NULL_HANDLE;
            }
            destroyed_ = true;
        }
    }

    VkDescriptorSetLayout getLayout() const { return layout_; }
    VkDescriptorPool getPool() const { return pool_; }
    VkDescriptorSet getSet() const { return set_; }

private:
    VkDevice device_{ VK_NULL_HANDLE };
    VkDescriptorSetLayout layout_{ VK_NULL_HANDLE };
    VkDescriptorPool pool_{ VK_NULL_HANDLE };
    VkDescriptorSet set_{ VK_NULL_HANDLE };
    bool destroyed_{ false };
};
