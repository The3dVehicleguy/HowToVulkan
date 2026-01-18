// AllocatorWrapper.h
#pragma once

#include <vma/vk_mem_alloc.h>

class AllocatorWrapper 
{
public:
    AllocatorWrapper() = default;
    ~AllocatorWrapper() = default;

    VmaAllocator Get() const noexcept { return allocator_; }

private:
    VmaAllocator allocator_{ VK_NULL_HANDLE };
};
