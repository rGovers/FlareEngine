#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "Rendering/TextureSampler.h"

class VulkanGraphicsEngine;
class VulkanPipeline;
class VulkanRenderEngineBackend;
class VulkanRenderTexture;
class VulkanSwapchain;

class VulkanRenderCommand
{
private:
    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;
    VulkanSwapchain*           m_swapchain;

    bool                       m_flushed;

    uint32_t                   m_renderTexAddr;
    uint32_t                   m_materialAddr;

    vk::CommandBuffer          m_commandBuffer;

protected:

public:
    VulkanRenderCommand(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_buffer);
    ~VulkanRenderCommand();

    void Flush();

    inline uint32_t GetRenderTexutreAddr() const
    {
        return m_renderTexAddr;
    }
    VulkanRenderTexture* GetRenderTexture() const;

    inline uint32_t GetMaterialAddr() const
    {
        return m_materialAddr;
    }
    VulkanPipeline* GetPipeline() const;

    VulkanPipeline* BindMaterial(uint32_t a_materialAddr);

    void PushTexture(uint32_t a_slot, const TextureSampler& a_sampler) const;
    
    void BindRenderTexture(uint32_t a_renderTexAddr);
    
    void Blit(const VulkanRenderTexture* a_src, const VulkanRenderTexture* a_dst);
};