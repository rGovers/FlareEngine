#pragma once

#define VMA_VULKAN_VERSION 1000000
#include <vk_mem_alloc.h>

#include <vulkan/vulkan.hpp>

class VulkanRenderEngineBackend;

class VulkanUniformBuffer
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_bufferCount;
    uint32_t                   m_uniformSize;
    vk::Buffer*                m_buffers;
    VmaAllocation*             m_allocations;
protected:

public:
    VulkanUniformBuffer(VulkanRenderEngineBackend* a_engine, uint32_t a_uniformSize);
    ~VulkanUniformBuffer();

    void SetData(uint32_t a_index, const char* a_data);

    inline vk::Buffer GetBuffer(uint32_t a_index) const
    {
        return m_buffers[a_index];
    }
};