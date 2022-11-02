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
    VulkanRenderEngineBackend* m_engine;

    VulkanRenderPass*          m_renderPass;

    vk::SwapchainKHR           m_swapchain = nullptr;
    std::vector<vk::ImageView> m_imageViews;
      
    vk::SurfaceFormatKHR       m_surfaceFormat;

    glm::ivec2                 m_size;
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

    inline VulkanRenderPass* GetRenderPass() const
    {
        return m_renderPass;
    }
};