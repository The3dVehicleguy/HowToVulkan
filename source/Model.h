// Model.h
#pragma once

#include "VulkanApp.h" // for Vertex
#include "Buffer.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <string>
#include <vector>

class Model 
{
public:
    Model() = default;
    ~Model() = default;

    // Load an OBJ and create a single mapped host-visible buffer containing
    // vertices followed by indices. Returns true on success.
    bool LoadFromObj(const std::string& filename, VmaAllocator allocator, VkDevice device, const VmaAllocationCreateInfo& allocInfo);

    VkBuffer GetBuffer() const { return buffer_.get(); }
    VkDeviceSize GetVertexBufferSize() const { return vBufSize_; }
    VkDeviceSize GetIndexBufferSize() const { return iBufSize_; }
    uint32_t GetIndexCount() const { return indexCount_; }

    void Destroy(VkDevice device) { buffer_.destroy(); }

    // Helpers for vertex input setup
    static VkVertexInputBindingDescription BindingDescription();
    static std::vector<VkVertexInputAttributeDescription> AttributeDescriptions();

private:
    Buffer buffer_;
    VkDeviceSize vBufSize_{ 0 };
    VkDeviceSize iBufSize_{ 0 };
    uint32_t indexCount_{ 0 };
    std::vector<Vertex> vertices_;
    std::vector<uint16_t> indices_;
};
