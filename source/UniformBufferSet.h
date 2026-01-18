#pragma once
#include "VulkanApp.h" // for ShaderDataBuffer and VulkanApp::maxFramesInFlight
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <array>
#include <iostream>

// Manages a set of per-frame uniform buffers (ShaderDataBuffer). Uses VMA to
// allocate buffers that are host-visible and retrieves device addresses for
// shader device address push-constant usage.
class UniformBufferSet {
public:
    UniformBufferSet() = default;
    UniformBufferSet(VmaAllocator allocator, VkDevice device)
        : allocator_(allocator), device_(device)
    {
        create();
    }

    ~UniformBufferSet()
    {
        // best-effort cleanup; call destroy(device, allocator) explicitly before allocator/device teardown
        if (!destroyed_) {
            // nothing here
        }
    }

    void create()
    {
        if (!allocator_ || device_ == VK_NULL_HANDLE) return;
        for (size_t i = 0; i < VulkanApp::maxFramesInFlight; ++i) {
            VkBufferCreateInfo uBufferCI{ .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = sizeof(ShaderData), .usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT };
            VmaAllocationCreateInfo uBufferAllocCI{ .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT, .usage = VMA_MEMORY_USAGE_AUTO };
            VkResult r = vmaCreateBuffer(allocator_, &uBufferCI, &uBufferAllocCI, &buffers_[i].buffer, &buffers_[i].allocation, nullptr);
            if (r != VK_SUCCESS) {
                std::cerr << "UniformBufferSet: vmaCreateBuffer failed: " << r << '\n';
                continue;
            }
            vmaMapMemory(allocator_, buffers_[i].allocation, &buffers_[i].mapped);
            VkBufferDeviceAddressInfo uBufferBdaInfo{ .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = buffers_[i].buffer };
            buffers_[i].deviceAddress = vkGetBufferDeviceAddress(device_, &uBufferBdaInfo);
        }
    }

    void destroy(VkDevice device)
    {
        if (destroyed_) return;
        for (size_t i = 0; i < VulkanApp::maxFramesInFlight; ++i) {
            if (buffers_[i].allocation != VK_NULL_HANDLE) {
                vmaUnmapMemory(allocator_, buffers_[i].allocation);
                vmaDestroyBuffer(allocator_, buffers_[i].buffer, buffers_[i].allocation);
                buffers_[i].buffer = VK_NULL_HANDLE;
                buffers_[i].allocation = VK_NULL_HANDLE;
                buffers_[i].mapped = nullptr;
                buffers_[i].deviceAddress = 0;
            }
        }
        destroyed_ = true;
    }

    std::array<ShaderDataBuffer, VulkanApp::maxFramesInFlight>& buffers() { return buffers_; }

private:
    VmaAllocator allocator_{ VK_NULL_HANDLE };
    VkDevice device_{ VK_NULL_HANDLE };
    std::array<ShaderDataBuffer, VulkanApp::maxFramesInFlight> buffers_{};
    bool destroyed_{ false };
};
