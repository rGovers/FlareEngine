#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <set>

#include "AppWindow/AppWindow.h"
#include "Config.h"
#include "FlareNativeConfig.h"
#include "Logger.h"
#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Trace.h"

const static std::vector<const char*> ValidationLayers = 
{
    "VK_LAYER_KHRONOS_validation"
};

const static std::vector<const char*> DeviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT a_msgSeverity, VkDebugUtilsMessageTypeFlagsEXT a_msgType, const VkDebugUtilsMessengerCallbackDataEXT* a_callbackData, void* a_userData)
{
    switch (a_msgSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
    {
        Logger::Message(std::string("Vulkan Validation Layer: ") + a_callbackData->pMessage);

        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
    {
        Logger::Warning(std::string("Vulkan Validation Layer: ") + a_callbackData->pMessage);

        break;
    }
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
    {
        Logger::Error(std::string("Vulkan Validation Layer: ") + a_callbackData->pMessage);

        break;
    }
    }

    return VK_FALSE;
}

static bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& a_device)
{
    uint32_t extensionCount;
    std::ignore = a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<vk::ExtensionProperties> availableExtensions = std::vector<vk::ExtensionProperties>(extensionCount);
    std::ignore = a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    uint32_t requiredCount = (uint32_t)DeviceExtensions.size();

    for (const char* requiredExtension : DeviceExtensions)
    {
        for (const vk::ExtensionProperties& extension : availableExtensions)
        {
            if (strcmp(requiredExtension, extension.extensionName) == 0)
            {
                --requiredCount;

                break;
            }
        }
    }

    return requiredCount == 0;
}

static bool IsDeviceSuitable(const vk::Instance& a_instance, const vk::PhysicalDevice& a_device, AppWindow* a_window)
{
    if (!CheckDeviceExtensionSupport(a_device))
    {
        return false;
    }

    if (!a_window->IsHeadless())
    {
        const SwapChainSupportInfo info = VulkanSwapchain::QuerySwapChainSupport(a_device, a_window->GetSurface(a_instance));
        if (info.Formats.empty() || info.PresentModes.empty())
        {
            return false;
        }
    }
    

    return a_device.getFeatures().geometryShader;
}

static bool CheckValidationLayerSupport()
{
    uint32_t layerCount = 0;
    std::ignore = vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<vk::LayerProperties> availableLayers = std::vector<vk::LayerProperties>(layerCount);
    std::ignore = vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : ValidationLayers)
    {
        for (const vk::LayerProperties& properties : availableLayers)
        {
            if (strcmp(layerName, properties.layerName) == 0)
            {
                goto NextIter;
            }
        }

        return false;

NextIter:;
    }

    return true;
} 

static std::vector<const char*> GetRequiredExtensions(const AppWindow* a_window)
{
    std::vector<const char*> extensions = a_window->GetRequiredVulkanExtenions();

    if constexpr (VulkanEnableValidationLayers)
    {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VulkanRenderEngineBackend::VulkanRenderEngineBackend(RuntimeManager* a_runtime, RenderEngine* a_engine) : RenderEngineBackend(a_engine)
{
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        m_commandBuffers[i] = std::vector<vk::CommandBuffer>();
    }

    const RenderEngine* renderEngine = GetRenderEngine();
    AppWindow* window = renderEngine->m_window;

    std::vector<const char*> enabledLayers;

    const bool headless = window->IsHeadless();

    if constexpr (VulkanEnableValidationLayers)
    {
        assert(CheckValidationLayerSupport());

        for (const auto& iter : ValidationLayers)
        {
            enabledLayers.emplace_back(iter);
        }
    }

    const vk::ApplicationInfo appInfo = vk::ApplicationInfo
    (
        renderEngine->m_config->GetApplicationName().data(), 
        0U, 
        "FlareEngine", 
        VK_MAKE_VERSION(FLARENATIVE_VERSION_MAJOR, FLARENATIVE_VERSION_MINOR, 0), 
        VK_API_VERSION_1_0, 
        nullptr
    );

    const std::vector<const char*> reqExtensions = GetRequiredExtensions(window);

    constexpr vk::DebugUtilsMessengerCreateInfoEXT DebugCreateInfo = vk::DebugUtilsMessengerCreateInfoEXT
    (
        vk::DebugUtilsMessengerCreateFlagsEXT(), 
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
        DebugCallback
    );

    vk::InstanceCreateInfo createInfo = vk::InstanceCreateInfo
    (
        vk::InstanceCreateFlags(),
        &appInfo, 
        (uint32_t)enabledLayers.size(), 
        enabledLayers.data(), 
        (uint32_t)reqExtensions.size(), 
        reqExtensions.data(),
        nullptr
    );

    if constexpr (VulkanEnableValidationLayers)
    {
        createInfo.pNext = &DebugCreateInfo;
    }

    if (vk::createInstance(&createInfo, nullptr, &m_instance) != vk::Result::eSuccess)
    {
        Logger::Error("Failed to create Vulkan Instance");

        assert(0);
    }

    TRACE("Created Vulkan Instance");

    if constexpr (VulkanEnableValidationLayers)
    {
        if (m_instance.createDebugUtilsMessengerEXT(&DebugCreateInfo, nullptr, &m_messenger) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to create Vulkan Debug Printing");

            assert(0);
        }

        TRACE("Created Vulkan Debug Layer");
    }

    uint32_t deviceCount = 0;
    std::ignore = m_instance.enumeratePhysicalDevices(&deviceCount, nullptr);

    assert(deviceCount > 0);

    std::vector<vk::PhysicalDevice> devices = std::vector<vk::PhysicalDevice>(deviceCount);
    std::ignore = m_instance.enumeratePhysicalDevices(&deviceCount, devices.data());

    bool foundDevice = false;

    for (const vk::PhysicalDevice& device : devices)
    {
        if (IsDeviceSuitable(m_instance, device, window))
        {
            m_pDevice = device;

            foundDevice = true;

            break;
        }
    }

    assert(foundDevice);

    TRACE("Found Vulkan Physical Device");

    uint32_t queueFamilyCount = 0;
    m_pDevice.getQueueFamilyProperties(&queueFamilyCount, nullptr);

    std::vector<vk::QueueFamilyProperties> queueFamilies = std::vector<vk::QueueFamilyProperties>(queueFamilyCount);
    m_pDevice.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());
    
    for (uint32_t i = 0; i < queueFamilyCount; ++i)
    {
        if (queueFamilies[i].queueFlags & vk::QueueFlagBits::eGraphics)
        {
            m_graphicsQueueIndex = i;
        }
        
        if (!headless)
        {
            vk::Bool32 presentSupport = VK_FALSE;

            std::ignore = m_pDevice.getSurfaceSupportKHR(i, window->GetSurface(m_instance), &presentSupport);

            if (presentSupport)
            {
                if (i == m_graphicsQueueIndex && m_presentQueueIndex == -1)
                {
                    m_presentQueueIndex = i;
                }
                else
                {
                    m_presentQueueIndex = i;
                }
            }
        }
       
    }

    std::set<uint32_t> uniqueQueueFamilies;
    if (m_graphicsQueueIndex != -1)
    {
        uniqueQueueFamilies.emplace(m_graphicsQueueIndex);
    }
    if (m_presentQueueIndex != -1)
    {
        uniqueQueueFamilies.emplace(m_presentQueueIndex);
    }

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    constexpr float QueuePriority = 1.0f;
    for (const uint32_t queueFamily : uniqueQueueFamilies)
    {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamily, 1, &QueuePriority));
    }

    constexpr vk::PhysicalDeviceFeatures deviceFeatures;

    vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo
    (
        vk::DeviceCreateFlags(), 
        (uint32_t)queueCreateInfos.size(), 
        queueCreateInfos.data(), 
        0, 
        nullptr, 
        0, 
        nullptr
    );

    if (!headless)
    {
        deviceCreateInfo.enabledExtensionCount = (uint32_t)DeviceExtensions.size();
        deviceCreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();
    }

    if constexpr (VulkanEnableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = (uint32_t)ValidationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
    }

    if (m_pDevice.createDevice(&deviceCreateInfo, nullptr, &m_lDevice) != vk::Result::eSuccess)
    {
        Logger::Error("Failed to create Vulkan Logic Device");

        assert(0);
    }

    TRACE("Created Vulkan Device");

    VmaVulkanFunctions vulkanFunctions;
    memset(&vulkanFunctions, 0, sizeof(vulkanFunctions));
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo;
    memset(&allocatorCreateInfo, 0, sizeof(allocatorCreateInfo));
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
    allocatorCreateInfo.physicalDevice = m_pDevice;
    allocatorCreateInfo.device = m_lDevice;
    allocatorCreateInfo.instance = m_instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    if (vmaCreateAllocator(&allocatorCreateInfo, &m_allocator) != VK_SUCCESS)
    {
        Logger::Error("Failed to create Vulkan Allocator");

        assert(0);
    }

    TRACE("Created Vulkan Allocator");

    if (m_graphicsQueueIndex != -1)
    {
        m_lDevice.getQueue(m_graphicsQueueIndex, 0, &m_graphicsQueue);    
    }
    if (m_presentQueueIndex != -1)
    {
        m_lDevice.getQueue(m_presentQueueIndex, 0, &m_presentQueue);
    }

    TRACE("Got Vulkan Queues");

    constexpr vk::SemaphoreCreateInfo semaphoreInfo;
    constexpr vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo
    (
        vk::FenceCreateFlagBits::eSignaled
    );

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        if (m_lDevice.createSemaphore(&semaphoreInfo, nullptr, &m_imageAvailable[i]) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to create image semaphore");

            assert(0);
        }
        if (m_lDevice.createSemaphore(&semaphoreInfo, nullptr, &m_renderFinished[i]) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to create render semaphore");

            assert(0);
        }
        if (m_lDevice.createFence(&fenceInfo, nullptr, &m_inFlight[i]) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to create fence");

            assert(0);
        }
    }
    
    TRACE("Created Vulkan sync objects");

    const vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo
    (
        vk::CommandPoolCreateFlagBits::eTransient,
        m_graphicsQueueIndex
    );  

    if (m_lDevice.createCommandPool(&poolInfo, nullptr, &m_commandPool) != vk::Result::eSuccess)
    {
        Logger::Error("Failed to create command pool");

        assert(0);
    }

    m_graphicsEngine = new VulkanGraphicsEngine(a_runtime, this);
}
VulkanRenderEngineBackend::~VulkanRenderEngineBackend()
{
    AppWindow* window = GetRenderEngine()->m_window;

    TRACE("Begin Vulkan clean up");
    m_lDevice.waitIdle();

    TRACE("Destroy Command Pool");
    m_lDevice.destroyCommandPool(m_commandPool);

    delete m_graphicsEngine;
    if (m_swapchain != nullptr)
    {
        delete m_swapchain;
        m_swapchain = nullptr;
    }

    TRACE("Destroy Vulkan Sync Objects");
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        m_lDevice.destroySemaphore(m_imageAvailable[i]);
        m_lDevice.destroySemaphore(m_renderFinished[i]);
        m_lDevice.destroyFence(m_inFlight[i]);
    }
    
    TRACE("Destroy Vulkan Allocator");
    vmaDestroyAllocator(m_allocator);
    
    vk::SurfaceKHR surface = window->GetSurface(m_instance);
    if (surface != vk::SurfaceKHR(nullptr))
    {
        TRACE("Destroying Surface");
        m_instance.destroySurfaceKHR(surface);
    }

    TRACE("Destroying Devices");
    m_lDevice.destroy();

    if constexpr (VulkanEnableValidationLayers)
    {
        TRACE("Cleaning Vulkan Diagnostics");
        m_instance.destroyDebugUtilsMessengerEXT(m_messenger);
    }

    TRACE("Destroying Vulkan Instance");
    // TODO: Need to work out why this take longer to dispose the longer the app is open
    m_instance.destroy();

    TRACE("Vulkan cleaned up");
}

void VulkanRenderEngineBackend::Update(double a_delta, double a_time)
{
    Profiler::StartFrame("Render Setup");

    AppWindow* window = GetRenderEngine()->m_window;
    if (m_swapchain == nullptr)
    {
        m_swapchain = new VulkanSwapchain(this, window);
    }

    if (!m_swapchain->StartFrame(m_imageAvailable[m_currentFrame], m_inFlight[m_currentFrame], &m_imageIndex, a_delta, a_time))
    {
        Profiler::StopFrame();

        return;
    }

    std::vector<vk::CommandBuffer>& buffers = m_commandBuffers[m_currentFrame];

    uint32_t buffersSize = (uint32_t)buffers.size();
    if (buffersSize > 0)
    {
        // TODO: Find fix for first lot of buffers causing validation error
        m_lDevice.freeCommandBuffers(m_graphicsEngine->GetCommandPool(), buffersSize, buffers.data());
    }

    Profiler::StopFrame();

    Profiler::StartFrame("Render Update");

    buffers = m_graphicsEngine->Update(m_swapchain);
    buffersSize = (uint32_t)buffers.size();
    // If there is nothing to render no point doing anything
    if (buffersSize <= 0)
    {
        Profiler::StopFrame();

        return;
    }

    constexpr vk::PipelineStageFlags WaitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    vk::SubmitInfo submitInfo = vk::SubmitInfo
    (
        0,
        nullptr,
        WaitStages,
        buffersSize,
        buffers.data()
    );

    if (!window->IsHeadless())
    {
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &m_imageAvailable[m_currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_renderFinished[m_currentFrame];

        if (m_graphicsQueue.submit(1, &submitInfo, m_inFlight[m_currentFrame]) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to submit command");

            assert(0);
        }
    }
    else
    {
        if (m_swapchain->IsInitialized(m_currentFrame))
        {
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &m_renderFinished[m_currentFrame];
        }

        if (m_graphicsQueue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to submit command");

            assert(0);
        }
    }

    Profiler::StopFrame();

    Profiler::StartFrame("Render Present");

    m_swapchain->EndFrame(m_renderFinished[m_currentFrame], m_inFlight[m_currentFrame], m_imageIndex);

    m_currentFrame = (m_currentFrame + 1) % VulkanMaxFlightFrames;

    Profiler::StopFrame();
}

vk::CommandBuffer VulkanRenderEngineBackend::CreateCommandBuffer(vk::CommandBufferLevel a_level) const
{   
    const vk::CommandBufferAllocateInfo allocInfo = vk::CommandBufferAllocateInfo
    (
        m_commandPool,
        a_level,
        1
    );

    vk::CommandBuffer cmdBuffer;
    if (m_lDevice.allocateCommandBuffers(&allocInfo, &cmdBuffer) != vk::Result::eSuccess)
    {
        Logger::Error("Failed to Allocate Command Buffer");

        assert(0);
    }

    return cmdBuffer;
}
void VulkanRenderEngineBackend::DestroyCommandBuffer(const vk::CommandBuffer& a_buffer) const
{
    m_lDevice.freeCommandBuffers(m_commandPool, 1, &a_buffer);
}

vk::CommandBuffer VulkanRenderEngineBackend::BeginSingleCommand() const
{
    const vk::CommandBuffer cmdBuffer = CreateCommandBuffer(vk::CommandBufferLevel::ePrimary);

    constexpr vk::CommandBufferBeginInfo BufferBeginInfo = vk::CommandBufferBeginInfo
    (
        vk::CommandBufferUsageFlagBits::eOneTimeSubmit
    );

    std::ignore = cmdBuffer.begin(&BufferBeginInfo);

    return cmdBuffer;
}
void VulkanRenderEngineBackend::EndSingleCommand(const vk::CommandBuffer& a_buffer) const
{
    a_buffer.end();

    const vk::SubmitInfo submitInfo = vk::SubmitInfo
    (
        0, 
        nullptr, 
        nullptr,
        1, 
        &a_buffer
    );

    if (m_graphicsQueue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess)
    {
        Logger::Error("Failed to Submit Command");

        assert(0);
    }
    m_graphicsQueue.waitIdle();

    m_lDevice.freeCommandBuffers(m_commandPool, 1, &a_buffer);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance a_instance, const VkDebugUtilsMessengerCreateInfoEXT* a_createInfo, const VkAllocationCallbacks* a_allocator, VkDebugUtilsMessengerEXT* a_messenger)
{
    TRACE("Custom Vulkan Debug Initializer Called");

    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(a_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(a_instance, a_createInfo, a_allocator, a_messenger);
    }

    return VK_ERROR_UNKNOWN;
}
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance a_instance, VkDebugUtilsMessengerEXT a_messenger, const VkAllocationCallbacks* a_allocator)
{
    TRACE("Custom Vulkan Debug Destructor Called");

    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(a_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(a_instance, a_messenger, a_allocator);
    }
}