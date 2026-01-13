// Descriptor.cpp
#include "Descriptor.h"

#include <volk/volk.h>
#include <iostream>
#include <vector>

VkDescriptorSetLayout Descriptor::createLayout(VkDevice device, uint32_t bindingCount) const
{
	VkDescriptorSetLayoutBinding binding{};
	binding.binding = 0;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	binding.descriptorCount = bindingCount;
	binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	binding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.bindingCount = 1;
	ci.pBindings = &binding;

	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	VkResult r = vkCreateDescriptorSetLayout(device, &ci, nullptr, &layout);
	if (r != VK_SUCCESS) {
		std::cerr << "vkCreateDescriptorSetLayout failed: " << r << std::endl;
		return VK_NULL_HANDLE;
	}
	return layout;
}

VkDescriptorPool Descriptor::createPool(VkDevice device, uint32_t descriptorCount) const
{
	VkDescriptorPoolSize poolSize{};
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSize.descriptorCount = descriptorCount;

	VkDescriptorPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	ci.maxSets = 1;
	ci.poolSizeCount = 1;
	ci.pPoolSizes = &poolSize;

	VkDescriptorPool pool = VK_NULL_HANDLE;
	VkResult r = vkCreateDescriptorPool(device, &ci, nullptr, &pool);
	if (r != VK_SUCCESS) {
		std::cerr << "vkCreateDescriptorPool failed: " << r << std::endl;
		return VK_NULL_HANDLE;
	}
	return pool;
}

VkDescriptorSet Descriptor::allocateAndWrite(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout, const std::vector<VkDescriptorImageInfo>& imageInfos) const
{
	if (pool == VK_NULL_HANDLE) return VK_NULL_HANDLE;
	uint32_t variableDescCount = static_cast<uint32_t>(imageInfos.size());

	VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{};
	variableDescCountAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
	variableDescCountAI.descriptorSetCount = 1;
	variableDescCountAI.pDescriptorCounts = &variableDescCount;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext = &variableDescCountAI;
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &layout;

	VkResult r = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
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
	vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);

	return descriptorSet;
}
// Descriptor.cpp
#include "Descriptor.h"

// scaffold only
	#include <volk/volk.h>
	#include <iostream>
    #include <vector>
    
	VkDescriptorSetLayout Descriptor::createLayout(VkDevice device, uint32_t bindingCount) const
	{
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = 0;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = bindingCount;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.pImmutableSamplers = nullptr;
    
		VkDescriptorSetLayoutCreateInfo ci{};
		ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		ci.bindingCount = 1;
		ci.pBindings = &binding;
    
		VkDescriptorSetLayout layout = VK_NULL_HANDLE;
		VkResult r = vkCreateDescriptorSetLayout(device, &ci, nullptr, &layout);
		if (r != VK_SUCCESS) {
			std::cerr << "vkCreateDescriptorSetLayout failed: " << r << std::endl;
			return VK_NULL_HANDLE;
		}

		VkDescriptorPool Descriptor::createPool(VkDevice device, uint32_t descriptorCount) const
		{
			VkDescriptorPoolSize poolSize{};
			poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSize.descriptorCount = descriptorCount;

			VkDescriptorPoolCreateInfo ci{};
			ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			ci.maxSets = 1;
			ci.poolSizeCount = 1;
			ci.pPoolSizes = &poolSize;

			// Descriptor.cpp
			#include "Descriptor.h"

			#include <volk/volk.h>
			#include <iostream>
			#include <vector>

			VkDescriptorSetLayout Descriptor::createLayout(VkDevice device, uint32_t bindingCount) const
			{
			    VkDescriptorSetLayoutBinding binding{};
			    binding.binding = 0;
			    binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			    binding.descriptorCount = bindingCount;
			    binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
			    binding.pImmutableSamplers = nullptr;

			    VkDescriptorSetLayoutCreateInfo ci{};
			    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			    ci.bindingCount = 1;
			    ci.pBindings = &binding;

			    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
			    VkResult r = vkCreateDescriptorSetLayout(device, &ci, nullptr, &layout);
			    if (r != VK_SUCCESS) {
			        std::cerr << "vkCreateDescriptorSetLayout failed: " << r << std::endl;
			        return VK_NULL_HANDLE;
			    }
			    return layout;
			}

			VkDescriptorPool Descriptor::createPool(VkDevice device, uint32_t descriptorCount) const
			{
			    VkDescriptorPoolSize poolSize{};
			    poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			    poolSize.descriptorCount = descriptorCount;

			    VkDescriptorPoolCreateInfo ci{};
			    ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			    ci.maxSets = 1;
			    ci.poolSizeCount = 1;
			    ci.pPoolSizes = &poolSize;

			    VkDescriptorPool pool = VK_NULL_HANDLE;
			    VkResult r = vkCreateDescriptorPool(device, &ci, nullptr, &pool);
			    if (r != VK_SUCCESS) {
			        std::cerr << "vkCreateDescriptorPool failed: " << r << std::endl;
			        return VK_NULL_HANDLE;
			    }
			    return pool;
			}

			VkDescriptorSet Descriptor::allocateAndWrite(VkDevice device, VkDescriptorPool pool, VkDescriptorSetLayout layout, const std::vector<VkDescriptorImageInfo>& imageInfos) const
			{
			    if (pool == VK_NULL_HANDLE) return VK_NULL_HANDLE;
			    uint32_t variableDescCount = static_cast<uint32_t>(imageInfos.size());

			    VkDescriptorSetVariableDescriptorCountAllocateInfo variableDescCountAI{};
			    variableDescCountAI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT;
			    variableDescCountAI.descriptorSetCount = 1;
			    variableDescCountAI.pDescriptorCounts = &variableDescCount;

			    VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
			    VkDescriptorSetAllocateInfo allocInfo{};
			    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			    allocInfo.pNext = &variableDescCountAI;
			    allocInfo.descriptorPool = pool;
			    allocInfo.descriptorSetCount = 1;
			    allocInfo.pSetLayouts = &layout;

			    VkResult r = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet);
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
			    vkUpdateDescriptorSets(device, 1, &writeDescSet, 0, nullptr);

			    return descriptorSet;
			}
