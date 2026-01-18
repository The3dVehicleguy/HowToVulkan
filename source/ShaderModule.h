#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

class ShaderModule {
public:
    ShaderModule() = default;
    ShaderModule(VkDevice device, const void* code, size_t codeSize);
    ~ShaderModule();

    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;
    ShaderModule(ShaderModule&& other) noexcept;
    ShaderModule& operator=(ShaderModule&& other) noexcept;

    VkShaderModule get() const { return module_; }
    bool valid() const { return module_ != VK_NULL_HANDLE; }

private:
    VkDevice device_{ VK_NULL_HANDLE };
    VkShaderModule module_{ VK_NULL_HANDLE };
};
