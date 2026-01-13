// Pipeline.h
#pragma once

#include <vulkan/vulkan.h>
#include <vector>

class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline() = default;

    // Create a graphics pipeline using the provided shader module (used for both
    // vertex and fragment stages), the pipeline layout, vertex input descriptions
    // and the color/depth formats. Returns VK_NULL_HANDLE on failure.
    VkPipeline createGraphics(
        VkDevice device,
        VkPipelineLayout layout,
        VkShaderModule shaderModule,
        const VkVertexInputBindingDescription& vertexBinding,
        const std::vector<VkVertexInputAttributeDescription>& vertexAttributes,
        VkFormat colorFormat,
        VkFormat depthFormat) const;
};
