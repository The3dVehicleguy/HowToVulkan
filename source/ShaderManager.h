#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include "ShaderModule.h"

// Small manager that owns one or more ShaderModule objects along with their
// corresponding shader stage flags. This allows Pipeline to accept a single
// object that may contain multiple stages (vertex, fragment, etc.).
class ShaderManager {
public:
    ShaderManager() = default;
    // Convenience ctor: use the same SPIR-V blob for both vertex and fragment
    // stages (matches the original sample behavior).
    ShaderManager(VkDevice device, const void* spirvCode, size_t codeSize);

    // General ctor: provide stages and blobs (ownership copied into modules)
    ShaderManager(VkDevice device, const std::vector<std::pair<VkShaderStageFlagBits, std::pair<const void*, size_t>>>& stages);

    ~ShaderManager() = default;

    size_t stageCount() const { return stages_.size(); }
    VkShaderStageFlagBits stageAt(size_t i) const { return stages_[i]; }
    VkShaderModule moduleAt(size_t i) const { return modules_[i].get(); }

private:
    std::vector<VkShaderStageFlagBits> stages_{};
    std::vector<ShaderModule> modules_{};
};
