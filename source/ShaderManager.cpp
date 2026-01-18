#include "ShaderManager.h"
#include <utility>

ShaderManager::ShaderManager(VkDevice device, const void* spirvCode, size_t codeSize)
{
    // Use the same module for vertex and fragment stages by default.
    stages_.reserve(2);
    modules_.reserve(2);
    stages_.push_back(VK_SHADER_STAGE_VERTEX_BIT);
    modules_.emplace_back(device, spirvCode, codeSize);
    stages_.push_back(VK_SHADER_STAGE_FRAGMENT_BIT);
    modules_.emplace_back(device, spirvCode, codeSize);
}

ShaderManager::ShaderManager(VkDevice device, const std::vector<std::pair<VkShaderStageFlagBits, std::pair<const void*, size_t>>>& stages)
{
    stages_.reserve(stages.size());
    modules_.reserve(stages.size());
    for (const auto& s : stages) {
        stages_.push_back(s.first);
        modules_.emplace_back(device, s.second.first, s.second.second);
    }
}
