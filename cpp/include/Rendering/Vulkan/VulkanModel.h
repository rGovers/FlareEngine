#pragma once

#define VMA_VULKAN_VERSION 1000000
#include <vk_mem_alloc.h>

#include <vulkan/vulkan.hpp>

class VulkanRenderEngineBackend;

class VulkanModel
{
private:
    VulkanRenderEngineBackend* m_engine;

    VmaAllocation              m_vbAlloc;
    VmaAllocation              m_ibAlloc;

    vk::Buffer                 m_vertexBuffer;
    vk::Buffer                 m_indexBuffer;

    uint32_t                   m_indexCount;

protected:

public:
    VulkanModel(VulkanRenderEngineBackend* a_engine, uint32_t a_vertexCount, const char* a_vertices, uint16_t a_vertexSize, uint32_t a_indexCount, const uint32_t* a_indices);
    ~VulkanModel();

    inline uint32_t GetIndexCount() const
    {
        return m_indexCount;
    }

    void Bind(const vk::CommandBuffer& a_cmdBuffer) const;
};