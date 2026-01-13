// Swapchain.cpp
#include "Swapchain.h"
#include <volk/volk.h>
#include <vector>
#include <iostream>
#include <cstring>

static inline void chk(VkResult r) {
	if (r != VK_SUCCESS) {
		std::cerr << "Vulkan error: " << r << std::endl;
		exit((int)r);
	}
}

VkSwapchainKHR Swapchain::create(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t queueFamilyIndex, VmaAllocator allocator)
{
	static int createCallCount = 0;
	std::cerr << "Swapchain::create entered (this=" << this << ") call#" << ++createCallCount << "\n";

	// Query surface formats and pick a reasonable default.
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	if (formatCount == 0) {
		std::cerr << "No surface formats available" << std::endl;
		return VK_NULL_HANDLE;
	}
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
	VkSurfaceFormatKHR surfaceFormat = formats[0];
	for (auto &f : formats) {
		if (f.format == VK_FORMAT_B8G8R8A8_SRGB) { surfaceFormat = f; break; }
	}

	VkSurfaceCapabilitiesKHR caps{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &caps);

	// Verify the selected queue family supports presentation to this surface.
	VkBool32 presentSupported = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueFamilyIndex, surface, &presentSupported);
	if (!presentSupported) {
		std::cerr << "Selected queue family does not support presentation\n";
		return VK_NULL_HANDLE;
	}

	// Debug print surface/caps info to help diagnose swapchain creation failures.
	std::cerr << "Surface format chosen: " << surfaceFormat.format << ", extent: " << caps.currentExtent.width << "x" << caps.currentExtent.height
		<< ", minImageCount=" << caps.minImageCount << ", maxImageCount=" << caps.maxImageCount << std::endl;

	VkExtent2D extent = caps.currentExtent;
	if (extent.width == (uint32_t)-1) { extent = {640, 480}; }

	uint32_t imageCount = caps.minImageCount + 1;
	if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount) imageCount = caps.maxImageCount;

	VkSwapchainCreateInfoKHR ci{};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.surface = surface;
	ci.minImageCount = imageCount;
	ci.imageFormat = surfaceFormat.format;
	ci.imageColorSpace = surfaceFormat.colorSpace;
	ci.imageExtent = extent;
	ci.imageArrayLayers = 1;
	ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	ci.preTransform = caps.currentTransform;
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	ci.clipped = VK_TRUE;
	// If we already have a swapchain, pass it as 'oldSwapchain' to the
	// create info so the implementation can recycle resources safely.
	VkSwapchainKHR oldSwap = swapchain_;
	if (oldSwap != VK_NULL_HANDLE) {
		ci.oldSwapchain = oldSwap;
	}

	VkSwapchainKHR newSwap = VK_NULL_HANDLE;
	VkResult r = vkCreateSwapchainKHR(device, &ci, nullptr, &newSwap);
	if (r != VK_SUCCESS) {
		std::cerr << "vkCreateSwapchainKHR failed: " << r << std::endl;
		std::cerr << "Swapchain::create (this=" << this << ") returning VK_NULL_HANDLE\n";
		return VK_NULL_HANDLE;
	}

	// Fetch images for the new swapchain first
	uint32_t imgCount = 0;
	vkGetSwapchainImagesKHR(device, newSwap, &imgCount, nullptr);
	std::vector<VkImage> newImages(imgCount);
	vkGetSwapchainImagesKHR(device, newSwap, &imgCount, newImages.data());

	// Create image views for the new images
	std::vector<VkImageView> newImageViews(imgCount, VK_NULL_HANDLE);
	for (uint32_t i = 0; i < imgCount; ++i) {
		VkImageViewCreateInfo viewCI{};
		viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCI.image = newImages[i];
		viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCI.format = surfaceFormat.format;
		viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCI.subresourceRange.levelCount = 1;
		viewCI.subresourceRange.layerCount = 1;
		chk(vkCreateImageView(device, &viewCI, nullptr, &newImageViews[i]));
	}

	// Create a new depth image for the new extent
	VkImage newDepthImage = VK_NULL_HANDLE;
	VmaAllocation newDepthAlloc = VK_NULL_HANDLE;
	VkImageView newDepthView = VK_NULL_HANDLE;
	VkImageCreateInfo depthImageCI{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = depthFormat_,
		.extent{.width = extent.width, .height = extent.height, .depth = 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};
	VmaAllocationCreateInfo allocCI{ .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT, .usage = VMA_MEMORY_USAGE_AUTO };
	chk(vmaCreateImage(allocator, &depthImageCI, &allocCI, &newDepthImage, &newDepthAlloc, nullptr));
	VkImageViewCreateInfo depthViewCI{};
	depthViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthViewCI.image = newDepthImage;
	depthViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthViewCI.format = depthFormat_;
	depthViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depthViewCI.subresourceRange.levelCount = 1;
	depthViewCI.subresourceRange.layerCount = 1;
	chk(vkCreateImageView(device, &depthViewCI, nullptr, &newDepthView));

	// At this point the new swapchain and its images/views/depth exist. Now
	// we can safely destroy old resources (if any) and update our members.
	if (oldSwap != VK_NULL_HANDLE) {
		// Destroy old image views
		for (auto &iv : imageViews_) { if (iv != VK_NULL_HANDLE) vkDestroyImageView(device, iv, nullptr); }
		// Destroy old depth resources
		if (depthView_ != VK_NULL_HANDLE) { vkDestroyImageView(device, depthView_, nullptr); depthView_ = VK_NULL_HANDLE; }
		if (depthImage_ != VK_NULL_HANDLE) { vmaDestroyImage(allocator, depthImage_, depthAlloc_); depthImage_ = VK_NULL_HANDLE; depthAlloc_ = VK_NULL_HANDLE; }
		// Destroy old swapchain handle
		if (swapchain_ != VK_NULL_HANDLE) { vkDestroySwapchainKHR(device, swapchain_, nullptr); }
	}

	// Update internal state to the newly created resources
	swapchain_ = newSwap;
	images_ = std::move(newImages);
	imageViews_ = std::move(newImageViews);
	imageFormat_ = surfaceFormat.format;
	extent_ = extent;
	depthImage_ = newDepthImage;
	depthAlloc_ = newDepthAlloc;
	depthView_ = newDepthView;

	std::cerr << "Swapchain::create (this=" << this << ") succeeded, swapchain_=" << swapchain_ << "\n";
	return swapchain_;
}

VkSwapchainKHR Swapchain::recreate(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, uint32_t queueFamilyIndex, VmaAllocator allocator)
{
	// Simply create will destroy old resources first; reuse create implementation
	return create(physicalDevice, device, surface, queueFamilyIndex, allocator);
}

void Swapchain::destroy(VkDevice device, VmaAllocator allocator)
{
	if (depthView_ != VK_NULL_HANDLE) { vkDestroyImageView(device, depthView_, nullptr); depthView_ = VK_NULL_HANDLE; }
	if (depthImage_ != VK_NULL_HANDLE) { vmaDestroyImage(allocator, depthImage_, depthAlloc_); depthImage_ = VK_NULL_HANDLE; depthAlloc_ = VK_NULL_HANDLE; }
	for (auto &iv : imageViews_) { if (iv != VK_NULL_HANDLE) vkDestroyImageView(device, iv, nullptr); }
	imageViews_.clear();
	images_.clear();
	if (swapchain_ != VK_NULL_HANDLE) { vkDestroySwapchainKHR(device, swapchain_, nullptr); swapchain_ = VK_NULL_HANDLE; }
}
