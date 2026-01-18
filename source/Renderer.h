// Renderer.h
#pragma once

#include "RenderContext.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <SFML/Graphics.hpp>
#include "VulkanApp.h" // for ShaderDataBuffer, Texture, Vertex types
#include <vector>
#include <array>

class Swapchain; // forward

class Renderer 
{
public:
    Renderer() = default;
    ~Renderer() = default;
    // Run the renderer loop using the provided RenderContext. The function
    // returns when the window is closed; it does not own the resources in
    // the context (ownership remains with the caller).
    int Run(const RenderContext& ctx);
};
