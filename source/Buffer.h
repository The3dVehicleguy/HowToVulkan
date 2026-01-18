// Buffer.h
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

class Buffer {
public:
    Buffer() = default;
    Buffer(VmaAllocator allocator, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, const VmaAllocationCreateInfo& allocInfo);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    VkBuffer get() const { return buffer_; }
    VmaAllocation allocation() const { return allocation_; }
    void* map();
    void unmap();
    VkDeviceAddress deviceAddress();

    bool valid() const { return buffer_ != VK_NULL_HANDLE; }

    // Explicitly free underlying VMA resources before allocator destruction
    void destroy();

private:
    VmaAllocator allocator_{ VK_NULL_HANDLE };
    VkDevice device_{ VK_NULL_HANDLE };
    VkBuffer buffer_{ VK_NULL_HANDLE };
    VmaAllocation allocation_{ VK_NULL_HANDLE };
    void* mapped_{ nullptr };
    VkDeviceAddress deviceAddress_{ 0 };
};
