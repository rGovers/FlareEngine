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

    vk::CommandBuffer          m_commandBuffer;

    vk::Semaphore              m_lastSemaphore;

    glm::ivec2                 m_renderSize;

protected:

public:
    VulkanRenderCommand(VulkanRenderEngineBackend* a_engine, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_buffer);
    ~VulkanRenderCommand();

    void Bind(VulkanRenderTexture* a_renderTexture, uint32_t a_index);

    inline uint32_t GetRenderTexutreAddr() const
    {
        return m_renderTexAddr;
    }

    inline glm::ivec2 GetRenderSize() const
    {
        return m_renderSize;
    }
};