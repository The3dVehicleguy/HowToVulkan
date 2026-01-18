// VulkanApp.h
// Small C++20 wrapper for the Vulkan sample application.
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
// VulkanApp.h
// Small C++20 wrapper for the Vulkan sample application.
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include <cstdint>

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct ShaderData {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 model[3];
    glm::vec4 lightPos{ 0.0f, -10.0f, 10.0f, 0.0f };
    uint32_t selected{ 1 };
};

struct ShaderDataBuffer {
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VkBuffer buffer{ VK_NULL_HANDLE };
    VkDeviceAddress deviceAddress{};
    void* mapped{ nullptr };
};

struct Texture {
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VkImage image{ VK_NULL_HANDLE };
    VkImageView view{ VK_NULL_HANDLE };
    VkSampler sampler{ VK_NULL_HANDLE };
};

class VulkanApp {
public:
    static constexpr uint32_t maxFramesInFlight = 2;

    VulkanApp(int argc, char* argv[]);
    ~VulkanApp();

    // Run the application. Returns process exit code.
    int Run();

private:
    int argc_{};
    char** argv_{};
};
