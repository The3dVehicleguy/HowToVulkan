// InstanceWrapper.h
#pragma once
// InstanceWrapper.h
#pragma once

#include <vulkan/vulkan.h>
#include <string>

// Lightweight RAII wrapper for VkInstance (minimal implementation)
class InstanceWrapper {
public:
    explicit InstanceWrapper(const std::string& appName = "HowToVulkan");
    ~InstanceWrapper();

    VkInstance get() const noexcept { return instance_; }

    // Non-copyable
    InstanceWrapper(const InstanceWrapper&) = delete;
    InstanceWrapper& operator=(const InstanceWrapper&) = delete;

    // Movable
    InstanceWrapper(InstanceWrapper&& other) noexcept;
    InstanceWrapper& operator=(InstanceWrapper&& other) noexcept;

private:
    VkInstance instance_{ VK_NULL_HANDLE };
};
