#include "ShaderModule.h"
#include <volk/volk.h>
#include <iostream>

ShaderModule::ShaderModule(VkDevice device, const void* code, size_t codeSize)
    : device_(device)
{
    if (!device_ || !code || codeSize == 0) return;
    VkShaderModuleCreateInfo ci{ .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = codeSize, .pCode = reinterpret_cast<const uint32_t*>(code) };
    VkResult res = vkCreateShaderModule(device_, &ci, nullptr, &module_);
    if (res != VK_SUCCESS) {
        std::cerr << "vkCreateShaderModule failed: " << res << "\n";
        module_ = VK_NULL_HANDLE;
    }
}

ShaderModule::~ShaderModule()
{
    if (module_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device_, module_, nullptr);
    }
}

ShaderModule::ShaderModule(ShaderModule&& other) noexcept
    : device_(other.device_), module_(other.module_)
{
    other.device_ = VK_NULL_HANDLE;
    other.module_ = VK_NULL_HANDLE;
}

ShaderModule& ShaderModule::operator=(ShaderModule&& other) noexcept
{
    if (this != &other) {
        if (module_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
            vkDestroyShaderModule(device_, module_, nullptr);
        }
        device_ = other.device_;
        module_ = other.module_;
        other.device_ = VK_NULL_HANDLE;
        other.module_ = VK_NULL_HANDLE;
    }
    return *this;
}
