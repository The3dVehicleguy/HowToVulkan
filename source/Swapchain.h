// Swapchain.h
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <vector>

class Swapchain 
{
public:
    Swapchain() = default;
    ~Swapchain() = default;

    // Create the swapchain and associated image views and depth buffer.
    // Returns the created VkSwapchainKHR or VK_NULL_HANDLE on failure.
    VkSwapchainKHR Create(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t queueFamilyIndex, VmaAllocator allocator);

    // Recreate the swapchain (destroys previous images/views/depth and creates new ones).
    // Recreate the swapchain: waits for device idle, refreshes surface caps,
    // creates a new swapchain and replaces internal resources safely.
    VkSwapchainKHR Recreate(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t queueFamilyIndex, VmaAllocator allocator);

    // Destroy all resources owned by this helper.
    void Destroy(VkDevice device, VmaAllocator allocator);

    // Accessors
    VkSwapchainKHR Get() const { return swapchain_; }
    std::vector<VkImage>& Images() { return images_; }
    std::vector<VkImageView>& ImageViews() { return imageViews_; }
    VkImage GetDepthImage() const { return depthImage_; }
    VmaAllocation GetDepthAllocation() const { return depthAlloc_; }
    VkImageView GetDepthView() const { return depthView_; }
    VkFormat GetImageFormat() const { return imageFormat_; }
    VkFormat GetDepthFormat() const { return depthFormat_; }
    VkExtent2D GetExtent() const { return extent_; }

private:
    VkSwapchainKHR swapchain_{ VK_NULL_HANDLE };
    std::vector<VkImage> images_;
    std::vector<VkImageView> imageViews_;
    VkImage depthImage_{ VK_NULL_HANDLE };
    VmaAllocation depthAlloc_{ VK_NULL_HANDLE };
    VkImageView depthView_{ VK_NULL_HANDLE };
    VkFormat imageFormat_{ VK_FORMAT_B8G8R8A8_SRGB };
    VkFormat depthFormat_{ VK_FORMAT_D24_UNORM_S8_UINT };
    VkExtent2D extent_{ 0, 0 };
};
