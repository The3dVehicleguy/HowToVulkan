// CommandPool.cpp
#include "CommandPool.h"

#include <volk/volk.h>
#include <vector>
#include <iostream>
#include <utility>

CommandPool::CommandPool(VkDevice device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
{
	VkCommandPoolCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	ci.queueFamilyIndex = queueFamilyIndex;
	ci.flags = flags;

	VkResult r = vkCreateCommandPool(device, &ci, nullptr, &pool_);
	if (r != VK_SUCCESS) {
		std::cerr << "vkCreateCommandPool failed: " << r << std::endl;
		pool_ = VK_NULL_HANDLE;
	}
	// remember device for RAII destruction
	device_ = device;
}

CommandPool::~CommandPool()
{
	destroy();
}

CommandPool::CommandPool(CommandPool&& other) noexcept
	: pool_(std::exchange(other.pool_, VK_NULL_HANDLE))
	, device_(std::exchange(other.device_, VK_NULL_HANDLE))
{
}

CommandPool& CommandPool::operator=(CommandPool&& other) noexcept
{
	if (this != &other) {
		destroy();
		pool_ = std::exchange(other.pool_, VK_NULL_HANDLE);
		device_ = std::exchange(other.device_, VK_NULL_HANDLE);
	}
	return *this;
}

void CommandPool::destroy()
{
	if (device_ != VK_NULL_HANDLE && pool_ != VK_NULL_HANDLE) {
		vkDestroyCommandPool(device_, pool_, nullptr);
		pool_ = VK_NULL_HANDLE;
		device_ = VK_NULL_HANDLE;
	}
}

std::vector<VkCommandBuffer> CommandPool::allocate(VkDevice device, uint32_t count) const
{
	if (pool_ == VK_NULL_HANDLE) return {};
	std::vector<VkCommandBuffer> buffers(count);
	VkCommandBufferAllocateInfo ai{};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.commandPool = pool_;
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	ai.commandBufferCount = count;

	VkResult r = vkAllocateCommandBuffers(device, &ai, buffers.data());
	if (r != VK_SUCCESS) {
		std::cerr << "vkAllocateCommandBuffers failed: " << r << std::endl;
		return {};
	}
	return buffers;
}
