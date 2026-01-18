// PhysicalDevice.h
#pragma once
// PhysicalDevice.h
#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

// Simple helper for physical device selection
class PhysicalDevice {
public:
    PhysicalDevice() = default;
    ~PhysicalDevice() = default;

    // Choose a physical device from the available list. Returns VK_NULL_HANDLE on failure.
    VkPhysicalDevice Choose(VkInstance instance, uint32_t index = 0) const;
};
