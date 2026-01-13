// Renderer.h
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <SFML/Graphics.hpp>
#include "VulkanApp.h" // for ShaderDataBuffer, Texture, Vertex types
#include <vector>
#include <array>

class Swapchain; // forward

// A compact context object that collects the runtime objects the renderer
// needs. Passing this single struct simplifies the renderer signature and
// makes ownership boundaries clearer.
struct RenderContext {
    sf::RenderWindow* window = nullptr;
    VkPhysicalDevice physical = VK_NULL_HANDLE;
    VkSurfaceKHR surface = VK_NULL_HANDLE;
    uint32_t queueFamily = 0;
    VkDevice device = VK_NULL_HANDLE;
    VkQueue queue = VK_NULL_HANDLE;
    VmaAllocator allocator = VK_NULL_HANDLE;
    Swapchain* swapchain = nullptr;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSet descriptorSetTex = VK_NULL_HANDLE;
    VkBuffer vBuffer = VK_NULL_HANDLE;
    VkDeviceSize vBufSize = 0;
    VkDeviceSize indexCount = 0;
    std::array<ShaderDataBuffer, VulkanApp::maxFramesInFlight>* shaderDataBuffers = nullptr;
    std::array<VkCommandBuffer, VulkanApp::maxFramesInFlight>* commandBuffers = nullptr;
    std::array<VkFence, VulkanApp::maxFramesInFlight>* fences = nullptr;
    std::array<VkSemaphore, VulkanApp::maxFramesInFlight>* presentSemaphores = nullptr;
    std::vector<VkSemaphore>* renderSemaphores = nullptr;
    VkSurfaceCapabilitiesKHR* surfaceCaps = nullptr;
};

class Renderer {
public:
    Renderer() = default;
    ~Renderer() = default;
    // Run the renderer loop using the provided RenderContext. The function
    // returns when the window is closed; it does not own the resources in
    // the context (ownership remains with the caller).
    int run(const RenderContext& ctx);
};
