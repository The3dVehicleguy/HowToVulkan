// Pipeline.h
#pragma once

#include <vulkan/vulkan.h>
#include "ShaderManager.h"
#include <vector>

// Pipeline helper: creates pipelines from an input descriptor struct to make
// constructing different kinds of pipelines (graphics/compute) easier.
class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline() = default;

    // Descriptor grouping inputs required for creating a graphics pipeline.
    struct GraphicsCreateInfo {
        VkDevice device{ VK_NULL_HANDLE };
        VkPipelineLayout layout{ VK_NULL_HANDLE };
        const ShaderManager* shaderManager{ nullptr };
        VkVertexInputBindingDescription vertexBinding{};
        std::vector<VkVertexInputAttributeDescription> vertexAttributes{};
        VkFormat colorFormat{ VK_FORMAT_UNDEFINED };
        VkFormat depthFormat{ VK_FORMAT_UNDEFINED };
    };

    // Create a graphics pipeline using a single grouped input structure.
    // Returns VK_NULL_HANDLE on failure.
    VkPipeline createGraphics(const GraphicsCreateInfo& info) const;
};
