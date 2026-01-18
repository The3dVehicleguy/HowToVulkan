#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

// Simple RAII for pipeline + layout. Provides explicit destroy(device)
// to ensure deterministic ordering during teardown.
class OwnedPipeline {
public:
    OwnedPipeline() = default;
    OwnedPipeline(VkDevice device, VkPipeline pipeline, VkPipelineLayout layout)
        : device_(device), pipeline_(pipeline), layout_(layout) {}

    ~OwnedPipeline()
    {
        if (!destroyed_) {
            // best-effort cleanup; explicit destroy(device) preferred
        }
    }

    void destroy(VkDevice device)
    {
        if (!destroyed_) {
            if (pipeline_ != VK_NULL_HANDLE) {
                vkDestroyPipeline(device, pipeline_, nullptr);
                pipeline_ = VK_NULL_HANDLE;
            }
            if (layout_ != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(device, layout_, nullptr);
                layout_ = VK_NULL_HANDLE;
            }
            destroyed_ = true;
        }
    }

    VkPipeline getPipeline() const { return pipeline_; }
    VkPipelineLayout getLayout() const { return layout_; }

private:
    VkDevice device_{ VK_NULL_HANDLE };
    VkPipeline pipeline_{ VK_NULL_HANDLE };
    VkPipelineLayout layout_{ VK_NULL_HANDLE };
    bool destroyed_{ false };
};
