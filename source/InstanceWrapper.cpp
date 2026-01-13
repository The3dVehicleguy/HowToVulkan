// InstanceWrapper.cpp
#include "InstanceWrapper.h"
#include <volk/volk.h>
#include <vector>
#include <iostream>
// InstanceWrapper.cpp
#include "InstanceWrapper.h"
#include <volk/volk.h>
#include <vector>
#include <iostream>

InstanceWrapper::InstanceWrapper(const std::string& appName)
{
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = appName.c_str();
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "NoEngine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // Enable required surface extensions so windowing libraries (SFML) can
    // create platform-specific surfaces. On Windows we need VK_KHR_surface
    // and VK_KHR_win32_surface. If you later add runtime queries for
    // required extensions, prefer those instead of hard-coding.
    const char* extensions[] = { VK_KHR_SURFACE_EXTENSION_NAME, VK_KHR_WIN32_SURFACE_EXTENSION_NAME };
    createInfo.enabledExtensionCount = sizeof(extensions) / sizeof(extensions[0]);
    createInfo.ppEnabledExtensionNames = extensions;

    VkResult res = vkCreateInstance(&createInfo, nullptr, &instance_);
    if (res != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance: " << res << std::endl;
        instance_ = VK_NULL_HANDLE;
        return;
    }

    // Initialize volk instance-level function pointers
    volkLoadInstance(instance_);
}

InstanceWrapper::~InstanceWrapper()
{
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
        instance_ = VK_NULL_HANDLE;
    }
}

InstanceWrapper::InstanceWrapper(InstanceWrapper&& other) noexcept
    : instance_(other.instance_)
{
    other.instance_ = VK_NULL_HANDLE;
}

InstanceWrapper& InstanceWrapper::operator=(InstanceWrapper&& other) noexcept
{
    if (this != &other) {
        if (instance_ != VK_NULL_HANDLE) {
            vkDestroyInstance(instance_, nullptr);
        }
        instance_ = other.instance_;
        other.instance_ = VK_NULL_HANDLE;
    }
    return *this;
}
