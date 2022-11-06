#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <set>

#include "Config.h"
#include "FlareNativeConfig.h"
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
    if (a_msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        printf("Validation Layer: %s \n", a_callbackData->pMessage);
    }

    return VK_FALSE;
}

static bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& a_device)
{
    uint32_t extensionCount;
    a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<vk::ExtensionProperties> availableExtensions = std::vector<vk::ExtensionProperties>(extensionCount);
    a_device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    uint32_t requiredCount = (uint32_t)DeviceExtensions.size();

    for (const auto& requiredExtension : DeviceExtensions)
    {
        for (const auto& extension : availableExtensions)
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

static bool IsDeviceSuitable(const vk::PhysicalDevice& a_device, const vk::SurfaceKHR& a_surface)
{
    if (!CheckDeviceExtensionSupport(a_device))
    {
        return false;
    }

    const SwapChainSupportInfo info = VulkanSwapchain::QuerySwapChainSupport(a_device, a_surface);
    if (info.Formats.empty() || info.PresentModes.empty())
    {
        return false;
    }

    return a_device.getFeatures().geometryShader;
}

static bool CheckValidationLayerSupport()
{
    uint32_t layerCount = 0;
    vk::enumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<vk::LayerProperties> availableLayers = std::vector<vk::LayerProperties>(layerCount);
    vk::enumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : ValidationLayers)
    {
        for (const auto& properties : availableLayers)
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

std::vector<const char*> VulkanRenderEngineBackend::GetRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions = std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if constexpr (EnableValidationLayers)
    {
        extensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

VulkanRenderEngineBackend::VulkanRenderEngineBackend(RuntimeManager* a_runtime, RenderEngine* a_engine) : RenderEngineBackend(a_engine)
{
    std::vector<const char*> enabledLayers;

    if constexpr (EnableValidationLayers)
    {
        assert(CheckValidationLayerSupport());

        for (const auto& iter : ValidationLayers)
        {
            enabledLayers.emplace_back(iter);
        }
    }

    const vk::ApplicationInfo appInfo = vk::ApplicationInfo
    (
        m_renderEngine->m_config->GetApplicationName().begin(), 
        0U, 
        "FlareEngine", 
        VK_MAKE_VERSION(FLARENATIVE_VERSION_MAJOR, FLARENATIVE_VERSION_MINOR, 0), 
        VK_API_VERSION_1_0, 
        nullptr
    );

    const std::vector<const char*> reqExtensions = GetRequiredExtensions();

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

    if constexpr (EnableValidationLayers)
    {
        createInfo.pNext = &DebugCreateInfo;
    }

    if (vk::createInstance(&createInfo, nullptr, &m_instance) != vk::Result::eSuccess)
    {
        printf("Failed to create Vulkan Instance \n");

        assert(0);
    }

    TRACE("Created Vulkan Instance");

    if constexpr (EnableValidationLayers)
    {
        if (m_instance.createDebugUtilsMessengerEXT(&DebugCreateInfo, nullptr, &m_messenger) != vk::Result::eSuccess)
        {
            printf("Failed to create Vulkan Debug Printing \n");

            assert(0);
        }
    }

    VkSurfaceKHR tempSurf;
    glfwCreateWindowSurface(m_instance, m_renderEngine->m_window, nullptr, &tempSurf);
    m_surface = vk::SurfaceKHR(tempSurf);

    TRACE("Created Vulkan Surface");

    uint32_t deviceCount = 0;
    m_instance.enumeratePhysicalDevices(&deviceCount, nullptr);

    assert(deviceCount > 0);

    std::vector<vk::PhysicalDevice> devices = std::vector<vk::PhysicalDevice>(deviceCount);
    m_instance.enumeratePhysicalDevices(&deviceCount, devices.data());

    bool foundDevice = false;

    for (const auto& device : devices)
    {
        if (IsDeviceSuitable(device, m_surface))
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

        vk::Bool32 presentSupport = VK_FALSE;
        m_pDevice.getSurfaceSupportKHR(i, m_surface, &presentSupport);
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

    const std::set<uint32_t> uniqueQueueFamilies = { m_graphicsQueueIndex, m_presentQueueIndex };

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;

    constexpr float QueuePriority = 1.0f;
    for (const uint32_t queueFamily : uniqueQueueFamilies)
    {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), queueFamily, 1, &QueuePriority));
    }

    constexpr vk::PhysicalDeviceFeatures deviceFeatures;

    vk::DeviceCreateInfo deviceCreateInfo = vk::DeviceCreateInfo(vk::DeviceCreateFlags(), (uint32_t)queueCreateInfos.size(), queueCreateInfos.data(), 0, nullptr, (uint32_t)DeviceExtensions.size(), DeviceExtensions.data());
    if constexpr (EnableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = (uint32_t)ValidationLayers.size();
        deviceCreateInfo.ppEnabledLayerNames = ValidationLayers.data();
    }

    if (m_pDevice.createDevice(&deviceCreateInfo, nullptr, &m_lDevice) != vk::Result::eSuccess)
    {
        printf("Failed to create Vulkan Logic Device \n");

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
        printf("Failed to create Vulkan Allocator \n");

        assert(0);
    }

    TRACE("Created Vulkan Allocator");

    m_lDevice.getQueue(m_graphicsQueueIndex, 0, &m_graphicsQueue);    
    m_lDevice.getQueue(m_presentQueueIndex, 0, &m_presentQueue);

    TRACE("Got Vulkan Queues");

    constexpr vk::SemaphoreCreateInfo semaphoreInfo;
    constexpr vk::FenceCreateInfo fenceInfo = vk::FenceCreateInfo
    (
        vk::FenceCreateFlagBits::eSignaled
    );

    if (m_lDevice.createSemaphore(&semaphoreInfo, nullptr, &m_imageAvailable) != vk::Result::eSuccess)
    {
        printf("Failed to create image semphore \n");

        assert(0);
    }
    if (m_lDevice.createSemaphore(&semaphoreInfo, nullptr, &m_renderFinished) != vk::Result::eSuccess)
    {
        printf("Failed to create render semaphore \n");

        assert(0);
    }
    if (m_lDevice.createFence(&fenceInfo, nullptr, &m_inFlight) != vk::Result::eSuccess)
    {
        printf("Failed to create fence \n");

        assert(0);
    }

    TRACE("Created Vulkan sync objects");

    m_graphicsEngine = new VulkanGraphicsEngine(a_runtime, this);
}
VulkanRenderEngineBackend::~VulkanRenderEngineBackend()
{
    TRACE("Begin Vulkan clean up");
    m_lDevice.waitIdle();

    delete m_graphicsEngine;
    if (m_swapchain != nullptr)
    {
        delete m_swapchain;
        m_swapchain = nullptr;
    }

    TRACE("Destroy Vulkan Sync Objects");
    m_lDevice.destroySemaphore(m_imageAvailable);
    m_lDevice.destroySemaphore(m_renderFinished);
    m_lDevice.destroyFence(m_inFlight);

    TRACE("Destroy Vulkan Allocator")
    vmaDestroyAllocator(m_allocator);

    TRACE("Destroying Surface");
    m_instance.destroySurfaceKHR(m_surface);

    TRACE("Destroying Devices");
    m_lDevice.destroy();

    if constexpr (EnableValidationLayers)
    {
        TRACE("Cleaning Vulkan Diagnostics");
        m_instance.destroyDebugUtilsMessengerEXT(m_messenger);
    }

    TRACE("Destroying Vulkan Instace");
    m_instance.destroy();

    TRACE("Vulkan cleaned up");
}

void VulkanRenderEngineBackend::Update()
{
    m_lDevice.waitForFences(1, &m_inFlight, VK_TRUE, UINT64_MAX);
    m_lDevice.resetFences(1, &m_inFlight);

    glm::ivec2 newWinSize;
    glfwGetWindowSize(m_renderEngine->m_window, &newWinSize.x, &newWinSize.y);

    if (m_swapchain == nullptr)
    {
        m_swapchain = new VulkanSwapchain(this, newWinSize);
    }
    else if (newWinSize != m_swapchain->GetSize())
    {
        printf("Not implemented \n");
    }

    m_lDevice.acquireNextImageKHR(m_swapchain->GetSwapchain(), UINT64_MAX, m_imageAvailable, nullptr, &m_imageIndex);

    const std::vector<vk::CommandBuffer> buffers = m_graphicsEngine->Update(m_swapchain);

    const vk::Semaphore waitSemaphores[] = { m_imageAvailable };
    const vk::Semaphore signalSemaphores[] = { m_renderFinished };
    constexpr vk::PipelineStageFlags WaitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

    const vk::SubmitInfo submitInfo = vk::SubmitInfo
    (
        1, 
        waitSemaphores, 
        WaitStages,
        (uint32_t)buffers.size(),
        buffers.data(),
        1,
        signalSemaphores
    );

    if (m_graphicsQueue.submit(1, &submitInfo, m_inFlight) != vk::Result::eSuccess)
    {
        printf("Failed to submit command \n");

        assert(0);
    }

    const vk::SwapchainKHR swapChains[] = { m_swapchain->GetSwapchain() };

    const vk::PresentInfoKHR presentInfo = vk::PresentInfoKHR
    (
        1,
        signalSemaphores,
        1,
        swapChains,
        &m_imageIndex
    );

    m_presentQueue.presentKHR(&presentInfo);
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