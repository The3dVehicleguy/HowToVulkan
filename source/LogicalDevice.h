// LogicalDevice.h
#pragma once

#include <vulkan/vulkan.h>

// Scaffold for a LogicalDevice wrapper
class LogicalDevice {
public:
    LogicalDevice() = default;
    ~LogicalDevice() = default;

    // Create a logical device from a physical device (scaffold)
    // Create a logical device from a physical device. Returns VK_NULL_HANDLE on failure.
    VkDevice create(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex = 0) const;
};
