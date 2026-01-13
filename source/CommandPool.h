// CommandPool.h
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class CommandPool {
public:
    CommandPool() = default;
    // RAII constructor - creates a command pool for the given device/queue family.
    CommandPool(VkDevice device, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
    ~CommandPool();

    // Non-copyable
    CommandPool(const CommandPool&) = delete;
    CommandPool& operator=(const CommandPool&) = delete;

    // Movable
    CommandPool(CommandPool&&) noexcept = default;
    CommandPool& operator=(CommandPool&&) noexcept = default;

    // Allocate `count` primary command buffers from the owned pool.
    std::vector<VkCommandBuffer> allocate(VkDevice device, uint32_t count) const;

    // Access underlying VkCommandPool
    VkCommandPool getPool() const { return pool_; }

private:
    VkCommandPool pool_{ VK_NULL_HANDLE };
    // Store device so we can destroy the pool in the destructor (RAII)
    VkDevice device_{ VK_NULL_HANDLE };
};
