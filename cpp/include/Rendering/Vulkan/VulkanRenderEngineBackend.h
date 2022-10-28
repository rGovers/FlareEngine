#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#define VMA_VULKAN_VERSION 1000000
#include <vk_mem_alloc.h>

#include "Rendering/RenderEngineBackend.h"

class VulkanRenderEngineBackend : public RenderEngineBackend
{
private:

#ifdef NDEBUG
    static constexpr bool EnableValidationLayers = false;
#else
    static constexpr bool EnableValidationLayers = true;
#endif

    VmaAllocator               m_allocator;

    vk::Instance               m_instance;
    vk::DebugUtilsMessengerEXT m_messenger;
    vk::PhysicalDevice         m_pDevice;
    vk::Device                 m_lDevice;
    
    vk::Queue                  m_graphicsQueue;
    vk::Queue                  m_presentQueue;

    vk::SurfaceKHR             m_surface;
    vk::SwapchainKHR           m_swapchain = nullptr;
    std::vector<vk::ImageView> m_imageViews;

    vk::SurfaceFormatKHR       m_surfaceFormat;

    uint32_t                   m_graphicsQueueIndex = -1;
    uint32_t                   m_presentQueueIndex = -1;

    glm::ivec2                 m_winSize = glm::ivec2(-1);

    std::vector<const char*> GetRequiredExtensions() const;

    void GenerateSwapChain();
    void GenerateSwapImages();
protected:

public:
    VulkanRenderEngineBackend(RenderEngine* a_engine);
    virtual ~VulkanRenderEngineBackend();

    virtual void Update();
};