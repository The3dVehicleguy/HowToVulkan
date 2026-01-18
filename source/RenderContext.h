#ifndef D9DB5386_A37E_4AEE_8210_537AE8080A6F
#define D9DB5386_A37E_4AEE_8210_537AE8080A6F

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
    struct Renderable {
        VkDescriptorSet descriptorSet{ VK_NULL_HANDLE };
        VkBuffer buffer{ VK_NULL_HANDLE };
        VkDeviceSize vertexByteSize{ 0 };
        VkDeviceSize indexByteSize{ 0 };
        uint32_t indexCount{ 0 };
    };
    std::vector<Renderable>* renderables = nullptr; // optional: if provided, renderer will draw these
    std::array<ShaderDataBuffer, VulkanApp::maxFramesInFlight>* shaderDataBuffers = nullptr;
    std::array<VkCommandBuffer, VulkanApp::maxFramesInFlight>* commandBuffers = nullptr;
    // Sync objects are managed by SyncObjects (vectors sized at runtime)
    std::vector<VkFence>* fences = nullptr;
    std::vector<VkSemaphore>* presentSemaphores = nullptr;
    std::vector<VkSemaphore>* renderSemaphores = nullptr;
    VkSurfaceCapabilitiesKHR* surfaceCaps = nullptr;
};


#endif /* D9DB5386_A37E_4AEE_8210_537AE8080A6F */
