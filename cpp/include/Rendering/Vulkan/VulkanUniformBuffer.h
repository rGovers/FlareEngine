#pragma once

#define VMA_VULKAN_VERSION 1000000
#include <vk_mem_alloc.h>

#include <vulkan/vulkan.hpp>

#include "Rendering/Vulkan/VulkanConstants.h"

class VulkanRenderEngineBackend;

class VulkanUniformBuffer
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_uniformSize;
    vk::Buffer                 m_buffers[VulkanMaxFlightFrames];
    VmaAllocation              m_allocations[VulkanMaxFlightFrames];
protected:

public:
    VulkanUniformBuffer(VulkanRenderEngineBackend* a_engine, uint32_t a_uniformSize);
    ~VulkanUniformBuffer();

    void SetData(uint32_t a_index, const void* a_data);

    inline vk::Buffer GetBuffer(uint32_t a_index) const
    {
        return m_buffers[a_index];
    }
    inline uint32_t GetSize() const
    {
        return m_uniformSize;
    }
};