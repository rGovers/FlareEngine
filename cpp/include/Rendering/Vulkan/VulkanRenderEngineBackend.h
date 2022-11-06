#pragma once

#include <vulkan/vulkan.hpp>

#define VMA_VULKAN_VERSION 1000000
#include <vk_mem_alloc.h>

#include "Rendering/RenderEngineBackend.h"

class RuntimeManager;
class VulkanGraphicsEngine;
class VulkanSwapchain;

class VulkanRenderEngineBackend : public RenderEngineBackend
{
private:
#ifdef NDEBUG
    static constexpr bool EnableValidationLayers = false;
#else
    static constexpr bool EnableValidationLayers = true;
#endif
    static constexpr uint32_t MaxFlightFrames = 2;

    VulkanGraphicsEngine*      m_graphicsEngine;
    VulkanSwapchain*           m_swapchain = nullptr;

    VmaAllocator               m_allocator;

    vk::Instance               m_instance;
    vk::DebugUtilsMessengerEXT m_messenger;
    vk::PhysicalDevice         m_pDevice;
    vk::Device                 m_lDevice;
          
    vk::Queue                  m_graphicsQueue;
    vk::Queue                  m_presentQueue;
      
    vk::SurfaceKHR             m_surface;

    std::vector<vk::Semaphore> m_imageAvailable = std::vector<vk::Semaphore>(MaxFlightFrames);
    std::vector<vk::Semaphore> m_renderFinished = std::vector<vk::Semaphore>(MaxFlightFrames);
    std::vector<vk::Fence>     m_inFlight = std::vector<vk::Fence>(MaxFlightFrames);

    uint32_t                   m_imageIndex = -1;
    uint32_t                   m_currentFrame = 0;

    uint32_t                   m_graphicsQueueIndex = -1;
    uint32_t                   m_presentQueueIndex = -1;

    std::vector<const char*> GetRequiredExtensions() const;

protected:

public:
    VulkanRenderEngineBackend(RuntimeManager* a_runtime, RenderEngine* a_engine);
    virtual ~VulkanRenderEngineBackend();

    virtual void Update();

    inline VmaAllocator GetAllocator() const
    {
        return m_allocator;
    }

    inline vk::Device GetLogicalDevice() const
    {
        return m_lDevice;
    }
    inline vk::PhysicalDevice GetPhysicalDevice() const
    {
        return m_pDevice;
    }

    inline vk::SurfaceKHR GetSurface() const
    {
        return m_surface;
    }

    inline uint32_t GetPresentQueueIndex() const
    {
        return m_presentQueueIndex;
    }
    inline uint32_t GetGraphicsQueueIndex() const
    {
        return m_graphicsQueueIndex;
    }
    inline vk::Queue GetPresentQueue() const
    {
        return m_presentQueue;
    }
    inline vk::Queue GetGraphicsQueue() const
    {
        return m_graphicsQueue;
    }

    inline uint32_t GetImageIndex() const
    {
        return m_imageIndex;
    }
};