#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

class VulkanRenderEngineBackend;
class VulkanRenderTexture;
class VulkanSwapchain;

class VulkanRenderCommand
{
private:
    VulkanRenderEngineBackend* m_engine;
    VulkanSwapchain*           m_swapchain;

    uint32_t                   m_renderTexAddr;
    VulkanRenderTexture*       m_renderTexture;

    vk::CommandBuffer          m_commandBuffer;

protected:

public:
    VulkanRenderCommand(VulkanRenderEngineBackend* a_engine, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_buffer);
    ~VulkanRenderCommand();

    inline bool IsTextureBound() const
    {
        return m_renderTexture != nullptr || (m_renderTexture == nullptr && m_renderTexAddr == -1);
    }

    void Flush();

    void Bind(VulkanRenderTexture* a_renderTexture, uint32_t a_index);
    
    void Blit(VulkanRenderTexture* a_src, VulkanRenderTexture* a_dst);

    inline uint32_t GetRenderTexutreAddr() const
    {
        return m_renderTexAddr;
    }
    inline VulkanRenderTexture* GetRenderTexture() const
    {
        return m_renderTexture;
    }
};