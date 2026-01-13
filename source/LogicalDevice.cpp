// LogicalDevice.cpp
#include "LogicalDevice.h"
#include <volk/volk.h>
#include <iostream>

VkDevice LogicalDevice::create(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex) const
{
        float queuePriority = 1.0f;
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;

    // Enable swapchain device extension so we can create a swapchain.
    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

        VkDevice device = VK_NULL_HANDLE;
        VkResult r = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
        if (r != VK_SUCCESS) {
            std::cerr << "vkCreateDevice failed: " << r << std::endl;
            return VK_NULL_HANDLE;
        }
        // Load device-level entrypoints for extensions via volk.
        volkLoadDevice(device);

        return device;
}
