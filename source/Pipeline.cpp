// Pipeline.cpp
#include "Pipeline.h"

#include <volk/volk.h>
#include <vector>
#include <iostream>

VkPipeline Pipeline::createGraphics(const GraphicsCreateInfo& info) const
{
    if (!info.shaderManager || info.device == VK_NULL_HANDLE) return VK_NULL_HANDLE;

    const ShaderManager& shaderManager = *info.shaderManager;

    // Shader stages (use provided shader manager)
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
    shaderStages.reserve(shaderManager.stageCount());
    for (size_t i = 0; i < shaderManager.stageCount(); ++i) {
        VkPipelineShaderStageCreateInfo sci{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
        sci.stage = shaderManager.stageAt(i);
        sci.module = shaderManager.moduleAt(i);
        sci.pName = "main";
        shaderStages.push_back(sci);
    }

    // Vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
    vertexInputState.vertexBindingDescriptionCount = 1;
    vertexInputState.pVertexBindingDescriptions = &info.vertexBinding;
    vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(info.vertexAttributes.size());
    vertexInputState.pVertexAttributeDescriptions = info.vertexAttributes.data();

    // Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // Dynamic states (viewport + scissor)
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Rasterization
    VkPipelineRasterizationStateCreateInfo rasterizationState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.lineWidth = 1.0f;

    // Multisample
    VkPipelineMultisampleStateCreateInfo multisampleState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Depth/stencil
    VkPipelineDepthStencilStateCreateInfo depthStencilState{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;

    // Color blend
    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask = 0xF;
    VkPipelineColorBlendStateCreateInfo colorBlendState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = &blendAttachment;

    // Rendering info (dynamic rendering)
    VkPipelineRenderingCreateInfo renderingCI{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
    renderingCI.colorAttachmentCount = 1;
    renderingCI.pColorAttachmentFormats = &info.colorFormat;
    renderingCI.depthAttachmentFormat = info.depthFormat;

    // Viewport state (no static viewport because we use dynamic state)
    VkPipelineViewportStateCreateInfo viewportState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkGraphicsPipelineCreateInfo pipelineCI{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineCI.pNext = &renderingCI;
    pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
    pipelineCI.pStages = shaderStages.data();
    pipelineCI.pVertexInputState = &vertexInputState;
    pipelineCI.pInputAssemblyState = &inputAssemblyState;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterizationState;
    pipelineCI.pMultisampleState = &multisampleState;
    pipelineCI.pDepthStencilState = &depthStencilState;
    pipelineCI.pColorBlendState = &colorBlendState;
    pipelineCI.pDynamicState = &dynamicState;
    pipelineCI.layout = info.layout;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkResult r = vkCreateGraphicsPipelines(info.device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline);
    if (r != VK_SUCCESS) {
        std::cerr << "vkCreateGraphicsPipelines failed: " << r << std::endl;
        return VK_NULL_HANDLE;
    }

    return pipeline;
}
