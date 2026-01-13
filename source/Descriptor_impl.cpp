// Descriptor_impl.cpp
#include "Descriptor.h"

#include <volk/volk.h>
#include <iostream>
#include <vector>


bool Descriptor::init(VkDevice device, uint32_t bindingCount, uint32_t descriptorCount)
{
    if (device == VK_NULL_HANDLE) return false;
    device_ = device;

    // Create layout
    VkDescriptorSetLayoutBinding binding{};
    binding.binding = 0;
    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    binding.descriptorCount = bindingCount;
    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutCI{};
    layoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCI.bindingCount = 1;
    layoutCI.pBindings = &binding;

    VkResult r = vkCreateDescriptorSetLayout(device_, &layoutCI, nullptr, &layout_);
    if (r != VK_SUCCESS) {
        std::cerr << "vkCreateDescriptorSetLayout failed: " << r << std::endl;
        return false;
    }

    // Create pool
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSize.descriptorCount = descriptorCount;

    VkDescriptorPoolCreateInfo poolCI{};
    poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCI.maxSets = 1;
    poolCI.poolSizeCount = 1;
    poolCI.pPoolSizes = &poolSize;

    r = vkCreateDescriptorPool(device_, &poolCI, nullptr, &pool_);
    if (r != VK_SUCCESS) {
        std::cerr << "vkCreateDescriptorPool failed: " << r << std::endl;
        vkDestroyDescriptorSetLayout(device_, layout_, nullptr);
        layout_ = VK_NULL_HANDLE;
        return false;
    }

    return true;
}

VkDescriptorSet Descriptor::allocateAndWrite(const std::vector<VkDescriptorImageInfo>& imageInfos) const
{
    if (device_ == VK_NULL_HANDLE || pool_ == VK_NULL_HANDLE || layout_ == VK_NULL_HANDLE) return VK_NULL_HANDLE;
    uint32_t variableDescCount = static_cast<uint32_t>(imageInfos.size());

    VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{};
    variableDescCountAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
    variableDescCountAI.descriptorSetCount = 1;
    variableDescCountAI.pDescriptorCounts = &variableDescCount;

    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = &variableDescCountAI;
    allocInfo.descriptorPool = pool_;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout_;

    VkResult r = vkAllocateDescriptorSets(device_, &allocInfo, &descriptorSet);
    if (r != VK_SUCCESS) {
        std::cerr << "vkAllocateDescriptorSets failed: " << r << std::endl;
        return VK_NULL_HANDLE;
    }

    VkWriteDescriptorSet writeDescSet{};
    writeDescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescSet.dstSet = descriptorSet;
    writeDescSet.dstBinding = 0;
    writeDescSet.descriptorCount = variableDescCount;
    writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writeDescSet.pImageInfo = imageInfos.data();
    vkUpdateDescriptorSets(device_, 1, &writeDescSet, 0, nullptr);

    return descriptorSet;
}

