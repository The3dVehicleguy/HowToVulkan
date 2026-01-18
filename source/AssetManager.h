// AssetManager.h
#pragma once

#include "Asset.h"
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>

class AssetManager 
{
public:
    AssetManager() = default;
    ~AssetManager() = default;

    // Add an asset by loading a model and its textures. Returns true on success.
    bool AddAsset(VkDevice device, VmaAllocator allocator, VkCommandPool cmdPool, VkQueue queue,
                  const std::string& modelFile, const std::vector<std::string>& textureFiles,
                  const VmaAllocationCreateInfo& modelAllocInfo)
    {
        assets_.emplace_back(std::make_unique<Asset>());
        if (!assets_.back()->Load(device, allocator, cmdPool, queue, modelFile, textureFiles, modelAllocInfo)) {
            assets_.pop_back();
            return false;
        }
        return true;
    }

    const Asset& GetAsset(size_t index) const { return *assets_.at(index); }
    Asset& GetAsset(size_t index) { return *assets_.at(index); }

    size_t Count() const { return assets_.size(); }

    void DestroyAll(VkDevice device, VmaAllocator allocator)
    {
        for (auto& a : assets_) a->Destroy(device, allocator);
        assets_.clear();
    }

private:
    std::vector<std::unique_ptr<Asset>> assets_;
};
