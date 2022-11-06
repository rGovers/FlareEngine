#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

class VulkanRenderEngineBackend;
class VulkanRenderPass;

struct SwapChainSupportInfo
{
    vk::SurfaceCapabilitiesKHR Capabilites;
    std::vector<vk::SurfaceFormatKHR> Formats;
    std::vector<vk::PresentModeKHR> PresentModes;
};

class VulkanSwapchain
{
private:
    VulkanRenderEngineBackend*   m_engine;
  
    vk::SwapchainKHR             m_swapchain = nullptr;
    vk::RenderPass               m_renderPass = nullptr;
    std::vector<vk::ImageView>   m_imageViews;
    std::vector<vk::Framebuffer> m_framebuffers;
      
    vk::SurfaceFormatKHR         m_surfaceFormat;

    glm::ivec2                   m_size;

    void Init(const glm::ivec2& a_size);
    void Destroy();
protected:

public:
    VulkanSwapchain(VulkanRenderEngineBackend* a_engine, const glm::ivec2& a_size);
    ~VulkanSwapchain();

    static SwapChainSupportInfo QuerySwapChainSupport(const vk::PhysicalDevice& a_device, const vk::SurfaceKHR& a_surface);

    inline vk::SurfaceFormatKHR GetSurfaceFormat() const
    {
        return m_surfaceFormat;
    }

    inline glm::ivec2 GetSize() const
    {
        return m_size;
    }

    inline vk::RenderPass GetRenderPass() const
    {
        return m_renderPass;
    }
    inline vk::Framebuffer GetFramebuffer(uint32_t a_index) const
    {
        return m_framebuffers[a_index];
    }
    inline vk::SwapchainKHR GetSwapchain() const
    {
        return m_swapchain;
    }

    void Rebuild(const glm::ivec2& a_size);
};