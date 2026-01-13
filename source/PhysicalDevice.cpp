// PhysicalDevice.cpp
#include "PhysicalDevice.h"
#include <volk/volk.h>
#include <iostream>

VkPhysicalDevice PhysicalDevice::choose(VkInstance instance, uint32_t index) const
{
    uint32_t deviceCount = 0;
    VkResult r = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (r != VK_SUCCESS || deviceCount == 0) {
        std::cerr << "No Vulkan physical devices found (" << r << ")" << std::endl;
        return VK_NULL_HANDLE;
    }
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    if (index >= deviceCount) {
        std::cerr << "Requested physical device index " << index << " out of range (" << deviceCount << ")" << std::endl;
        return VK_NULL_HANDLE;
    }
    return devices[index];
}
