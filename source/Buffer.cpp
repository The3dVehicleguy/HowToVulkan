// Buffer.cpp
#include "Buffer.h"
#include <cstring>
#include <volk/volk.h>

Buffer::Buffer(VmaAllocator allocator, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, const VmaAllocationCreateInfo& allocInfo)
    : allocator_(allocator), device_(device)
{
    VkBufferCreateInfo bufferCI{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = usage };
    vmaCreateBuffer(allocator_, &bufferCI, &allocInfo, &buffer_, &allocation_, nullptr);
}

Buffer::~Buffer()
{
    destroy();
}

Buffer::Buffer(Buffer&& other) noexcept
    : allocator_(other.allocator_), device_(other.device_), buffer_(other.buffer_), allocation_(other.allocation_), mapped_(other.mapped_), deviceAddress_(other.deviceAddress_)
{
    other.buffer_ = VK_NULL_HANDLE;
    other.allocation_ = VK_NULL_HANDLE;
    other.mapped_ = nullptr;
    other.deviceAddress_ = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
    if (this != &other) {
        destroy();
        allocator_ = other.allocator_;
        device_ = other.device_;
        buffer_ = other.buffer_;
        allocation_ = other.allocation_;
        mapped_ = other.mapped_;
        deviceAddress_ = other.deviceAddress_;

        other.buffer_ = VK_NULL_HANDLE;
        other.allocation_ = VK_NULL_HANDLE;
        other.mapped_ = nullptr;
        other.deviceAddress_ = 0;
    }
    return *this;
}

void* Buffer::map()
{
    if (!mapped_ && allocation_ != VK_NULL_HANDLE) {
        vmaMapMemory(allocator_, allocation_, &mapped_);
    }
    return mapped_;
}

void Buffer::unmap()
{
    if (mapped_ && allocation_ != VK_NULL_HANDLE) {
        vmaUnmapMemory(allocator_, allocation_);
        mapped_ = nullptr;
    }
}

VkDeviceAddress Buffer::deviceAddress()
{
    if (deviceAddress_ == 0 && buffer_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        VkBufferDeviceAddressInfo bdai{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = buffer_ };
        deviceAddress_ = vkGetBufferDeviceAddress(device_, &bdai);
    }
    return deviceAddress_;
}

void Buffer::destroy()
{
    if (buffer_ != VK_NULL_HANDLE) {
        if (mapped_ && allocation_ != VK_NULL_HANDLE) {
            vmaUnmapMemory(allocator_, allocation_);
            mapped_ = nullptr;
        }
        if (allocation_ != VK_NULL_HANDLE) {
            vmaDestroyBuffer(allocator_, buffer_, allocation_);
        } else {
            vmaDestroyBuffer(allocator_, buffer_, allocation_);
        }
        buffer_ = VK_NULL_HANDLE;
        allocation_ = VK_NULL_HANDLE;
        deviceAddress_ = 0;
    }
}
