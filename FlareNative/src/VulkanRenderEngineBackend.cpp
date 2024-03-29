#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <set>

#include "AppWindow/AppWindow.h"
#include "Config.h"
#include "Flare/FlareAssert.h"
#include "FlareNativeConfig.h"
#include "Logger.h"
#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

const static std::vector<const char*> ValidationLayers = 
{
    "VK_LAYER_KHRONOS_validation"
};

const static std::vector<const char*> InstanceExtensions = 
{
    VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
};
const static std::vector<const char*> DeviceExtensions = 
{
    VK_KHR_MAINTENANCE_3_EXTENSION_NAME,
    VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME
};

const static std::vector<const char*> StandaloneDeviceExtensions =
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

static bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& a_device, const std::vector<const char*>& a_extensions)
{
    uint32_t extensionCount;
    FLARE_ASSERT_R(a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr) == vk::Result::eSuccess);

    std::vector<vk::ExtensionProperties> availableExtensions = std::vector<vk::ExtensionProperties>(extensionCount);
    FLARE_ASSERT_R(a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data()) == vk::Result::eSuccess);

    uint32_t requiredCount = (uint32_t)a_extensions.size();

    for (const char* requiredExtension : a_extensions)
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

static bool IsDeviceSuitable(const vk::Instance& a_instance, const vk::PhysicalDevice& a_device, const std::vector<const char*>& a_extensions, AppWindow* a_window)
{
    // TODO: Improve device selection
    // Fix issue with laptops and multi gpu
    if (!CheckDeviceExtensionSupport(a_device, a_extensions))
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
    
    const vk::PhysicalDeviceFeatures features = a_device.getFeatures();

    return features.geometryShader && features.samplerAnisotropy;
}

static bool CheckValidationLayerSupport()
{
    uint32_t layerCount = 0;
    FLARE_ASSERT_R(vk::enumerateInstanceLayerProperties(&layerCount, nullptr) == vk::Result::eSuccess);

    std::vector<vk::LayerProperties> availableLayers = std::vector<vk::LayerProperties>(layerCount);
    FLARE_ASSERT_R(vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data()) == vk::Result::eSuccess);

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

    for (const char* ext : InstanceExtensions)
    {
        extensions.emplace_back(ext);
    }

    return extensions;
}

VulkanRenderEngineBackend::VulkanRenderEngineBackend(RuntimeManager* a_runtime, RenderEngine* a_engine) : RenderEngineBackend(a_engine)
{
    m_runtime = a_runtime;

    const RenderEngine* renderEngine = GetRenderEngine();
    AppWindow* window = renderEngine->m_window;

    std::vector<const char*> enabledLayers;

    const bool headless = window->IsHeadless();

    if constexpr (VulkanEnableValidationLayers)
    {
        FLARE_ASSERT_R(CheckValidationLayerSupport());

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
        FLARE_VULKAN_VERSION, 
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

    FLARE_ASSERT_MSG_R(vk::createInstance(&createInfo, nullptr, &m_instance) == vk::Result::eSuccess, "Failed to create Vulkan Instance");

    TRACE("Created Vulkan Instance");

    if constexpr (VulkanEnableValidationLayers)
    {
        FLARE_ASSERT_MSG_R(m_instance.createDebugUtilsMessengerEXT(&DebugCreateInfo, nullptr, &m_messenger) == vk::Result::eSuccess, "Failed to create Vulkan Debug Printing");

        TRACE("Created Vulkan Debug Layer");
    }

    std::vector<const char*> dRequiredExtensions = DeviceExtensions;
    if (!headless)
    {
        for (const char* ext : StandaloneDeviceExtensions)
        {
            dRequiredExtensions.emplace_back(ext);
        }
    }

    uint32_t deviceCount = 0;
    FLARE_ASSERT_R(m_instance.enumeratePhysicalDevices(&deviceCount, nullptr) == vk::Result::eSuccess);

    FLARE_ASSERT(deviceCount > 0);

    std::vector<vk::PhysicalDevice> devices = std::vector<vk::PhysicalDevice>(deviceCount);
    FLARE_ASSERT_R(m_instance.enumeratePhysicalDevices(&deviceCount, devices.data()) == vk::Result::eSuccess);

    bool foundDevice = false;

    for (const vk::PhysicalDevice& device : devices)
    {
        if (IsDeviceSuitable(m_instance, device, dRequiredExtensions, window))
        {
            m_pDevice = device;

            foundDevice = true;

            break;
        }
    }

    FLARE_ASSERT(foundDevice);

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

            FLARE_ASSERT_R(m_pDevice.getSurfaceSupportKHR(i, window->GetSurface(m_instance), &presentSupport) == vk::Result::eSuccess);

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

    vk::PhysicalDeviceFeatures deviceFeatures;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo
    (
        { }, 
        (uint32_t)queueCreateInfos.size(), 
        queueCreateInfos.data(), 
        0, 
        nullptr, 
        0, 
        nullptr,
        &deviceFeatures
    );

    deviceCreateInfo.enabledExtensionCount = (uint32_t)dRequiredExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = dRequiredExtensions.data();

    if constexpr (VulkanEnableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = (uint32_t)ValidationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
    }

    FLARE_ASSERT_MSG_R(m_pDevice.createDevice(&deviceCreateInfo, nullptr, &m_lDevice) == vk::Result::eSuccess, "Failed to create Vulkan Logic Device");

    TRACE("Created Vulkan Device");

    VmaVulkanFunctions vulkanFunctions;
    memset(&vulkanFunctions, 0, sizeof(vulkanFunctions));
    vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
    vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;

    VmaAllocatorCreateInfo allocatorCreateInfo;
    memset(&allocatorCreateInfo, 0, sizeof(allocatorCreateInfo));
    allocatorCreateInfo.vulkanApiVersion = FLARE_VULKAN_VERSION;
    allocatorCreateInfo.physicalDevice = m_pDevice;
    allocatorCreateInfo.device = m_lDevice;
    allocatorCreateInfo.instance = m_instance;
    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    FLARE_ASSERT_MSG_R(vmaCreateAllocator(&allocatorCreateInfo, &m_allocator) == VK_SUCCESS, "Failed to create Vulkan Allocator");

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

    constexpr vk::SemaphoreCreateInfo SemaphoreInfo;
    constexpr vk::FenceCreateInfo FenceInfo = vk::FenceCreateInfo
    (
        vk::FenceCreateFlagBits::eSignaled
    );

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        FLARE_ASSERT_MSG_R(m_lDevice.createSemaphore(&SemaphoreInfo, nullptr, &m_imageAvailable[i]) == vk::Result::eSuccess, "Failed to create image semaphore");
        FLARE_ASSERT_MSG_R(m_lDevice.createFence(&FenceInfo, nullptr, &m_inFlight[i]) == vk::Result::eSuccess, "Failed to create fence");
    }
    
    TRACE("Created Vulkan sync objects");

    const vk::CommandPoolCreateInfo poolInfo = vk::CommandPoolCreateInfo
    (
        vk::CommandPoolCreateFlagBits::eTransient,
        m_graphicsQueueIndex
    );  

    FLARE_ASSERT_MSG_R(m_lDevice.createCommandPool(&poolInfo, nullptr, &m_commandPool) == vk::Result::eSuccess, "Failed to create command pool");

    PFN_vkGetPhysicalDeviceProperties2KHR GetPhysicalDeviceProperties2KHRFunc = (PFN_vkGetPhysicalDeviceProperties2KHR)vkGetInstanceProcAddr(m_instance, "vkGetPhysicalDeviceProperties2KHR");

    VkPhysicalDevicePushDescriptorPropertiesKHR pushProperties = { };
    pushProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR;

    VkPhysicalDeviceProperties2KHR deviceProps2 = { };
    deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2_KHR;
	deviceProps2.pNext = &pushProperties;

    GetPhysicalDeviceProperties2KHRFunc(m_pDevice, &deviceProps2);
    m_pushDescriptorProperties = pushProperties;

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
        m_lDevice.destroyFence(m_inFlight[i]);

        for (uint32_t j = 0; j < m_interSemaphore[i].size(); ++j)
        {
            m_lDevice.destroySemaphore(m_interSemaphore[i][j]);
        }
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
    Profiler::StartFrame("Swap Setup");

    m_runtime->AttachThread();

    AppWindow* window = GetRenderEngine()->m_window;
    if (m_swapchain == nullptr)
    {
        m_swapchain = new VulkanSwapchain(this, window, m_runtime);
        m_graphicsEngine->SetSwapchain(m_swapchain);
    }

    if (!m_swapchain->StartFrame(m_imageAvailable[m_currentFlightFrame], m_inFlight[m_currentFlightFrame], &m_imageIndex, a_delta, a_time))
    {
        Profiler::StopFrame();

        return;
    }

    Profiler::StopFrame();

    Profiler::StartFrame("Render Update");

    const std::vector<vk::CommandBuffer> buffers = m_graphicsEngine->Update(m_currentFrame);
    
    Profiler::StartFrame("Render Setup");

    const uint32_t buffersSize = (uint32_t)buffers.size();
    // If there is nothing to render no point doing anything
    if (buffersSize <= 0)
    {
        Profiler::StopFrame();

        return;
    }

    constexpr vk::PipelineStageFlags WaitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    const uint32_t semaphoreCount = (uint32_t)m_interSemaphore[m_currentFlightFrame].size();
    const uint32_t endBuffer = buffersSize - 1;
    if (buffersSize > semaphoreCount)
    {
        TRACE("Allocating inter semaphores");
        const uint32_t diff = buffersSize - semaphoreCount;

        constexpr vk::SemaphoreCreateInfo SemaphoreInfo;

        for (uint32_t i = 0; i < diff; ++i)
        {
            vk::Semaphore semaphore;

            FLARE_ASSERT_MSG_R(m_lDevice.createSemaphore(&SemaphoreInfo, nullptr, &semaphore) == vk::Result::eSuccess, "Failed to create inter semaphore");
            m_interSemaphore[m_currentFlightFrame].emplace_back(semaphore);
        }
    }

    Profiler::StopFrame();

    Profiler::StartFrame("Render Submit");

    vk::Semaphore lastSemaphore = nullptr;
    vk::Fence fence = nullptr;
    if (!window->IsHeadless())
    {
        lastSemaphore = m_imageAvailable[m_currentFlightFrame];
        fence = m_inFlight[m_currentFlightFrame];
    }

    for (uint32_t i = 0; i < buffersSize; ++i)
    {
        vk::SubmitInfo submitInfo = vk::SubmitInfo
        (
            0,
            nullptr,
            WaitStages,
            1,
            &buffers[i],
            1,
            &m_interSemaphore[m_currentFlightFrame][i]
        );

        if (lastSemaphore != vk::Semaphore(nullptr))
        {
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &lastSemaphore;
        }

        if (i == endBuffer)
        {
            FLARE_ASSERT_MSG_R(m_graphicsQueue.submit(1, &submitInfo, fence) == vk::Result::eSuccess, "Failed to submit command");
        }
        else
        {
            FLARE_ASSERT_MSG_R(m_graphicsQueue.submit(1, &submitInfo, nullptr) == vk::Result::eSuccess, "Failed to submit command");
        }

        lastSemaphore = m_interSemaphore[m_currentFlightFrame][i];
    }    

    Profiler::StopFrame();

    Profiler::StopFrame();

    Profiler::StartFrame("Swap Present");

    m_swapchain->EndFrame(m_interSemaphore[m_currentFlightFrame][endBuffer], m_inFlight[m_currentFlightFrame], m_imageIndex);

    m_currentFrame = (m_currentFrame + 1) % VulkanFlightPoolSize;
    m_currentFlightFrame = (m_currentFlightFrame + 1) % VulkanMaxFlightFrames;

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
    FLARE_ASSERT_MSG_R(m_lDevice.allocateCommandBuffers(&allocInfo, &cmdBuffer) == vk::Result::eSuccess, "Failed to Allocate Command Buffer");

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

    FLARE_ASSERT_R(cmdBuffer.begin(&BufferBeginInfo) == vk::Result::eSuccess);

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

    FLARE_ASSERT_MSG_R(m_graphicsQueue.submit(1, &submitInfo, nullptr) == vk::Result::eSuccess, "Failed to Submit Command");

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