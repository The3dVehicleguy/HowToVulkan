// Model.cpp
#include "Model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <cstring>
#include <iostream>

bool Model::LoadFromObj(const std::string& filename, VmaAllocator allocator, VkDevice device, const VmaAllocationCreateInfo& allocInfo)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str())) 
    {
        std::cerr << "tinyobj: " << warn << " " << err << "\n";
        return false;
    }
    if (shapes.empty()) 
    {
        std::cerr << "Model::loadFromOBJ: no shapes in OBJ\n";
        return false;
    }

    vertices_.clear();
    indices_.clear();

    for (auto& index : shapes[0].mesh.indices) 
    {
        Vertex v{
            .pos = { attrib.vertices[index.vertex_index * 3], -attrib.vertices[index.vertex_index * 3 + 1], attrib.vertices[index.vertex_index * 3 + 2] },
            .normal = { attrib.normals[index.normal_index * 3], -attrib.normals[index.normal_index * 3 + 1], attrib.normals[index.normal_index * 3 + 2] },
            .uv = { attrib.texcoords[index.texcoord_index * 2], 1.0f - attrib.texcoords[index.texcoord_index * 2 + 1] }
        };
        vertices_.push_back(v);
        indices_.push_back(static_cast<uint16_t>(indices_.size()));
    }

    vBufSize_ = sizeof(Vertex) * vertices_.size();
    iBufSize_ = sizeof(uint16_t) * indices_.size();
    indexCount_ = static_cast<uint32_t>(indices_.size());

    // Create combined buffer
    buffer_ = Buffer(allocator, device, vBufSize_ + iBufSize_, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, allocInfo);
    void* ptr = buffer_.map();
    if (!ptr) 
    {
        std::cerr << "Model: failed to map buffer\n";
        return false;
    }
    std::memcpy(ptr, vertices_.data(), vBufSize_);
    std::memcpy(static_cast<char*>(ptr) + vBufSize_, indices_.data(), iBufSize_);
    buffer_.unmap();

    return true;
}

VkVertexInputBindingDescription Model::BindingDescription()
{
    VkVertexInputBindingDescription binding{ .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX };
    return binding;
}

std::vector<VkVertexInputAttributeDescription> Model::AttributeDescriptions()
{
    return std::vector<VkVertexInputAttributeDescription>{
        { .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, pos) },
        { .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, normal) },
        { .location = 2, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, uv) },
    };
}
