#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <set>

#include "Config.h"
#include "FlareNativeConfig.h"
#include "Rendering/RenderEngine.h"

const static std::vector<const char*> ValidationLayers = 
{
    "VK_LAYER_KHRONOS_validation"
};

const static std::vector<const char*> DeviceExtensions =
{
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupportInfo
{
    vk::SurfaceCapabilitiesKHR Capabilites;
    std::vector<vk::SurfaceFormatKHR> Formats;
    std::vector<vk::PresentModeKHR> PresentModes;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT a_msgSeverity, VkDebugUtilsMessageTypeFlagsEXT a_msgType, const VkDebugUtilsMessengerCallbackDataEXT* a_callbackData, void* a_userData)
{
    if (a_msgSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        printf("Validation Layer: %s \n", a_callbackData->pMessage);
    }

    return VK_FALSE;
}

static SwapChainSupportInfo QuerySwapChainSupport(const vk::PhysicalDevice& a_device, const vk::SurfaceKHR& a_surface)
{
    SwapChainSupportInfo info;

    a_device.getSurfaceCapabilitiesKHR(a_surface, &info.Capabilites);

    uint32_t formatCount;
    a_device.getSurfaceFormatsKHR(a_surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        info.Formats.resize(formatCount);
        a_device.getSurfaceFormatsKHR(a_surface, &formatCount, info.Formats.data());
    }

    uint32_t presentModeCount;
    a_device.getSurfacePresentModesKHR(a_surface, &presentModeCount, nullptr);
    if (presentModeCount != 0)
    {
        info.PresentModes.resize(presentModeCount);
        a_device.getSurfacePresentModesKHR(a_surface, &presentModeCount, info.PresentModes.data());
    }

    return info;
}

static vk::SurfaceFormatKHR GetSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& a_formats)
{
    for (const auto& format : a_formats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return format;
        }
    }

    return a_formats[0];
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

    SwapChainSupportInfo info = QuerySwapChainSupport(a_device, a_surface);
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

static vk::Extent2D GetSwapExtent(const vk::SurfaceCapabilitiesKHR& a_capabilities, const glm::ivec2& a_size)
{
    const vk::Extent2D minExtent = a_capabilities.minImageExtent;
    const vk::Extent2D maxExtent = a_capabilities.maxImageExtent;

    return vk::Extent2D(glm::clamp((uint32_t)a_size.x, minExtent.width, maxExtent.width), glm::clamp((uint32_t)a_size.y, minExtent.height, maxExtent.height));
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

void VulkanRenderEngineBackend::GenerateSwapChain()
{
    const SwapChainSupportInfo info = QuerySwapChainSupport(m_pDevice, m_surface);

    m_surfaceFormat = GetSurfaceFormat(info.Formats);
    constexpr vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    const vk::Extent2D extents = GetSwapExtent(info.Capabilites, m_winSize);

    uint32_t imageCount = info.Capabilites.minImageCount + 1;
    if (info.Capabilites.maxImageCount > 0)
    {
        imageCount = glm::min(imageCount, info.Capabilites.maxImageCount);
    }

    vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR
    (
        vk::SwapchainCreateFlagsKHR(), 
        m_surface, 
        imageCount, 
        m_surfaceFormat.format, 
        m_surfaceFormat.colorSpace, 
        extents, 
        1, 
        vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 
        nullptr,
        info.Capabilites.currentTransform,
        vk::CompositeAlphaFlagBitsKHR::eOpaque,
        presentMode,
        VK_TRUE,
        m_swapchain
    );

    const uint32_t queueFamilyIndices[] = { m_graphicsQueueIndex, m_presentQueueIndex };

    if (m_presentQueue != m_graphicsQueue)
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    if (m_lDevice.createSwapchainKHR(&createInfo, nullptr, &m_swapchain) != vk::Result::eSuccess)
    {
        printf("Failed to create Vulkan Swapchain \n");

        assert(0);
    }
}
void VulkanRenderEngineBackend::GenerateSwapImages()
{
    uint32_t imageCount;
    m_lDevice.getSwapchainImagesKHR(m_swapchain, &imageCount, nullptr);
    std::vector<vk::Image> swapImages = std::vector<vk::Image>(imageCount);
    m_lDevice.getSwapchainImagesKHR(m_swapchain, &imageCount, swapImages.data());

    m_imageViews.resize(imageCount);

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        vk::ImageViewCreateInfo createInfo = vk::ImageViewCreateInfo(vk::ImageViewCreateFlags(), swapImages[i], vk::ImageViewType::e2D, m_surfaceFormat.format);
        createInfo.subresourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        if (m_lDevice.createImageView(&createInfo, nullptr, &m_imageViews[i]) != vk::Result::eSuccess)
        {
            printf("Failed to create Swapchain Image View \n");

            assert(0);
        }
    }
}

VulkanRenderEngineBackend::VulkanRenderEngineBackend(RenderEngine* a_engine) : RenderEngineBackend(a_engine)
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

    m_lDevice.getQueue(m_graphicsQueueIndex, 0, &m_graphicsQueue);    
    m_lDevice.getQueue(m_presentQueueIndex, 0, &m_presentQueue);
}
VulkanRenderEngineBackend::~VulkanRenderEngineBackend()
{
    if constexpr (EnableValidationLayers)
    {
        m_instance.destroyDebugUtilsMessengerEXT(m_messenger);
    }

    for (auto imageView : m_imageViews)
    {
        m_lDevice.destroyImageView(imageView, nullptr);
    }

    if (m_winSize.x != -1 || m_winSize.y != -1)
    {
        vkDestroySwapchainKHR(m_lDevice, m_swapchain, nullptr);
    }
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    m_lDevice.destroy();
    m_instance.destroy();
}

void VulkanRenderEngineBackend::Update()
{
    glm::ivec2 newWinSize;
    glfwGetWindowSize(m_renderEngine->m_window, &newWinSize.x, &newWinSize.y);

    if (m_winSize.x == -1 || m_winSize.y == -1 || newWinSize != m_winSize)
    {
        m_winSize = newWinSize;

        GenerateSwapChain();
        GenerateSwapImages();
    }
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance a_instance, const VkDebugUtilsMessengerCreateInfoEXT* a_createInfo, const VkAllocationCallbacks* a_allocator, VkDebugUtilsMessengerEXT* a_messenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(a_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(a_instance, a_createInfo, a_allocator, a_messenger);
    }

    return VK_ERROR_UNKNOWN;
}
VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance a_instance, VkDebugUtilsMessengerEXT a_messenger, const VkAllocationCallbacks* a_allocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(a_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(a_instance, a_messenger, a_allocator);
    }
}