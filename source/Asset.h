// Asset.h
#pragma once

#include "Model.h"
#include "TextureHandle.h"
#include "DescriptorRAII.h"
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <string>
#include <vector>

class Asset 
{
public:
    Asset() = default;
    ~Asset() = default;

    // Load a model and associated textures. cmdPool and queue are used to
    // transfer/initialize texture images. Returns true on success.
    bool Load(VkDevice device, VmaAllocator allocator, VkCommandPool cmdPool, VkQueue queue, const std::string& modelFile,
              const std::vector<std::string>& textureFiles, const VmaAllocationCreateInfo& modelAllocInfo);

    // Destroy resources owned by this asset (textures, descriptors, model buffer)
    void Destroy(VkDevice device, VmaAllocator allocator);

    // Accessors
    VkBuffer GetModelBuffer() const { return model_.GetBuffer(); }
    VkDeviceSize GetModelVertexSize() const { return model_.GetVertexBufferSize(); }
    VkDeviceSize GetModelIndexSize() const { return model_.GetIndexBufferSize(); }
    uint32_t GetModelIndexCount() const { return model_.GetIndexCount(); }
    VkDescriptorSet GetDescriptorSet() const { return descriptorOwned_.getSet(); }
    VkDescriptorSetLayout GetDescriptorLayout() const { return descriptorOwned_.getLayout(); }

    // Vertex input helpers (forwarded to Model)
    VkVertexInputBindingDescription GetVertexBindingDescription() const { return Model::BindingDescription(); }
    std::vector<VkVertexInputAttributeDescription> GetVertexAttributeDescriptions() const { return Model::AttributeDescriptions(); }

private:
    VkDevice device_{ VK_NULL_HANDLE };
    Model model_;
    std::vector<TextureHandle> textures_;
    DescriptorRAII descriptorOwned_;
};
