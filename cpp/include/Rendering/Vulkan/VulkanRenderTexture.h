#pragma once

#include <vulkan/vulkan.hpp>

#define VMA_VULKAN_VERSION 1000000
#include <vk_mem_alloc.h>

class VulkanRenderEngineBackend;

class VulkanRenderTexture
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_textureCount;
                    
    uint32_t                   m_width;
    uint32_t                   m_height;

    bool                       m_hdr;

    vk::RenderPass             m_renderPass;
    vk::Framebuffer            m_frameBuffer;

    vk::Image*                 m_textures;
    vk::ImageView*             m_textureViews;
    VmaAllocation*             m_textureAllocations;

    vk::ClearValue*            m_clearValues;
    
    void Init(uint32_t a_width, uint32_t a_height);
    void Destroy();

protected:

public:
    VulkanRenderTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_textureCount, uint32_t a_width, uint32_t a_height, bool a_hdr = false);
    ~VulkanRenderTexture();

    inline uint32_t GetWidth() const
    {
        return m_width;
    }
    inline uint32_t GetHeight() const
    {
        return m_height;
    }

    inline vk::RenderPass GetRenderPass() const
    {
        return m_renderPass;
    }
    inline vk::Framebuffer GetFramebuffer() const
    {
        return m_frameBuffer;
    }

    inline uint32_t GetTextureCount() const
    {
        return m_textureCount;
    }

    inline vk::ClearValue* GetClearValues() const
    {
        return m_clearValues;
    }

    void Resize(uint32_t a_width, uint32_t a_height);
};