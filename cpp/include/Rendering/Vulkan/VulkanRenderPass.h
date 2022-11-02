#pragma once

#include <vulkan/vulkan.hpp>

class VulkanRenderEngineBackend;
class VulkanSwapchain;

class VulkanRenderPass
{
private:
    VulkanRenderEngineBackend* m_engine;

    vk::RenderPass             m_renderPass;
protected:

public:
    VulkanRenderPass(VulkanRenderEngineBackend* a_engine, const VulkanSwapchain* a_swapchain);
    ~VulkanRenderPass();

    inline vk::RenderPass GetRenderPass() const
    {
        return m_renderPass;
    }
};