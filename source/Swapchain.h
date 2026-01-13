// Swapchain.h
#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <vector>

class Swapchain {
public:
    Swapchain() = default;
    ~Swapchain() = default;

    // Create the swapchain and associated image views and depth buffer.
    // Returns the created VkSwapchainKHR or VK_NULL_HANDLE on failure.
    VkSwapchainKHR create(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t queueFamilyIndex, VmaAllocator allocator);

    // Recreate the swapchain (destroys previous images/views/depth and creates new ones).
    // Recreate the swapchain: waits for device idle, refreshes surface caps,
    // creates a new swapchain and replaces internal resources safely.
    VkSwapchainKHR recreate(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t queueFamilyIndex, VmaAllocator allocator);

    // Destroy all resources owned by this helper.
    void destroy(VkDevice device, VmaAllocator allocator);

    // Accessors
    VkSwapchainKHR get() const { return swapchain_; }
    std::vector<VkImage>& images() { return images_; }
    std::vector<VkImageView>& imageViews() { return imageViews_; }
    VkImage getDepthImage() const { return depthImage_; }
    VmaAllocation getDepthAllocation() const { return depthAlloc_; }
    VkImageView getDepthView() const { return depthView_; }
    VkFormat getImageFormat() const { return imageFormat_; }
    VkFormat getDepthFormat() const { return depthFormat_; }
    VkExtent2D getExtent() const { return extent_; }

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
