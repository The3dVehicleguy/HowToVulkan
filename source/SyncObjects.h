#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>
#include <iostream>

// Non-templated SyncObjects owning fences and semaphores. The caller should
// call destroy(device) before destroying the VkDevice to guarantee safe
// teardown ordering.
class SyncObjects {
public:
    SyncObjects() = default;
    SyncObjects(uint32_t framesInFlight, VkDevice device, uint32_t swapchainImageCount)
    {
        create(framesInFlight, device, swapchainImageCount);
    }

    ~SyncObjects()
    {
        if (!destroyed_ && device_ != VK_NULL_HANDLE) {
            // Best-effort cleanup; explicit destroy(device) is preferred.
            destroy(device_);
        }
    }

    void create(uint32_t framesInFlight, VkDevice device, uint32_t swapchainImageCount)
    {
        device_ = device;
        fences_.resize(framesInFlight, VK_NULL_HANDLE);
        presentSemaphores_.resize(framesInFlight, VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreCI{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VkFenceCreateInfo fenceCI{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };

        for (uint32_t i = 0; i < framesInFlight; ++i) {
            VkResult r = vkCreateFence(device_, &fenceCI, nullptr, &fences_[i]);
            if (r != VK_SUCCESS) { std::cerr << "vkCreateFence failed: " << r << '\n'; exit(r); }
            r = vkCreateSemaphore(device_, &semaphoreCI, nullptr, &presentSemaphores_[i]);
            if (r != VK_SUCCESS) { std::cerr << "vkCreateSemaphore failed: " << r << '\n'; exit(r); }
        }

        renderSemaphores_.resize(swapchainImageCount, VK_NULL_HANDLE);
        for (auto& s : renderSemaphores_) {
            VkResult r = vkCreateSemaphore(device_, &semaphoreCI, nullptr, &s);
            if (r != VK_SUCCESS) { std::cerr << "vkCreateSemaphore failed: " << r << '\n'; exit(r); }
        }
    }

    void destroy(VkDevice device)
    {
        for (auto& s : renderSemaphores_) {
            if (s != VK_NULL_HANDLE) {
                vkDestroySemaphore(device, s, nullptr);
                s = VK_NULL_HANDLE;
            }
        }
        renderSemaphores_.clear();

        for (auto& s : presentSemaphores_) {
            if (s != VK_NULL_HANDLE) {
                vkDestroySemaphore(device, s, nullptr);
                s = VK_NULL_HANDLE;
            }
        }

        for (auto& f : fences_) {
            if (f != VK_NULL_HANDLE) {
                vkDestroyFence(device, f, nullptr);
                f = VK_NULL_HANDLE;
            }
        }

        destroyed_ = true;
        device_ = VK_NULL_HANDLE;
    }

    std::vector<VkFence>& fences() { return fences_; }
    std::vector<VkSemaphore>& presentSemaphores() { return presentSemaphores_; }
    std::vector<VkSemaphore>& renderSemaphores() { return renderSemaphores_; }

private:
    VkDevice device_{ VK_NULL_HANDLE };
    std::vector<VkFence> fences_{};
    std::vector<VkSemaphore> presentSemaphores_{};
    std::vector<VkSemaphore> renderSemaphores_{};
    bool destroyed_{ false };
};
#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <vector>
#include <cstdint>

// Small helper to create fences and semaphores and populate caller-owned
// containers. Implemented as a header-only template for convenience.
template <size_t N>
inline void createSyncObjects(VkDevice device, uint32_t swapchainImageCount, std::array<VkFence, N>& fences, std::array<VkSemaphore, N>& presentSemaphores, std::vector<VkSemaphore>& renderSemaphores)
{
    VkSemaphoreCreateInfo semaphoreCI{ .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
    VkFenceCreateInfo fenceCI{ .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };

    for (size_t i = 0; i < N; ++i) {
        vkCreateFence(device, &fenceCI, nullptr, &fences[i]);
        vkCreateSemaphore(device, &semaphoreCI, nullptr, &presentSemaphores[i]);
    }
    renderSemaphores.resize(swapchainImageCount);
    for (auto& s : renderSemaphores) {
        vkCreateSemaphore(device, &semaphoreCI, nullptr, &s);
    }
}
