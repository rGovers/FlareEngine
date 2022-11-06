#include "Rendering/Vulkan/VulkanSwapchain.h"

#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

static vk::SurfaceFormatKHR GetSurfaceFormatFromFormats(const std::vector<vk::SurfaceFormatKHR>& a_formats)
{
    for (const vk::SurfaceFormatKHR& format : a_formats)
    {
        if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return format;
        }
    }

    return a_formats[0];
}

static vk::Extent2D GetSwapExtent(const vk::SurfaceCapabilitiesKHR& a_capabilities, const glm::ivec2& a_size)
{
    const vk::Extent2D minExtent = a_capabilities.minImageExtent;
    const vk::Extent2D maxExtent = a_capabilities.maxImageExtent;

    return vk::Extent2D(glm::clamp((uint32_t)a_size.x, minExtent.width, maxExtent.width), glm::clamp((uint32_t)a_size.y, minExtent.height, maxExtent.height));
}

void VulkanSwapchain::Init(const glm::ivec2& a_size)
{
    m_size = a_size;

    const vk::PhysicalDevice pDevice = m_engine->GetPhysicalDevice();
    const vk::SurfaceKHR surface = m_engine->GetSurface();
    const vk::Device lDevice = m_engine->GetLogicalDevice();

    const SwapChainSupportInfo info = QuerySwapChainSupport(pDevice, surface);

    m_surfaceFormat = GetSurfaceFormatFromFormats(info.Formats);
    constexpr vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    const vk::Extent2D extents = GetSwapExtent(info.Capabilites, m_size);

    uint32_t imageCount = info.Capabilites.minImageCount + 1;
    if (info.Capabilites.maxImageCount > 0)
    {
        imageCount = glm::min(imageCount, info.Capabilites.maxImageCount);
    }

    vk::SwapchainCreateInfoKHR createInfo = vk::SwapchainCreateInfoKHR
    (
        vk::SwapchainCreateFlagsKHR(), 
        surface, 
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

    const uint32_t queueFamilyIndices[] = { m_engine->GetGraphicsQueueIndex(), m_engine->GetPresentQueueIndex() };

    if (m_engine->GetGraphicsQueue() != m_engine->GetPresentQueue())
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }

    if (lDevice.createSwapchainKHR(&createInfo, nullptr, &m_swapchain) != vk::Result::eSuccess)
    {
        printf("Failed to create Vulkan Swapchain \n");

        assert(0);
    }
    TRACE("Created Vulkan Swapchain");

    const SwapChainSupportInfo info = QuerySwapChainSupport(pDevice, surface);

    m_surfaceFormat = GetSurfaceFormatFromFormats(info.Formats);
    constexpr vk::PresentModeKHR presentMode = vk::PresentModeKHR::eFifo;
    const vk::Extent2D extents = GetSwapExtent(info.Capabilites, m_size);

    uint32_t imageCount = info.Capabilites.minImageCount + 1;
    if (info.Capabilites.maxImageCount > 0)
    {
        imageCount = glm::min(imageCount, info.Capabilites.maxImageCount);
    }

    lDevice.getSwapchainImagesKHR(m_swapchain, &imageCount, nullptr);
    std::vector<vk::Image> swapImages = std::vector<vk::Image>(imageCount);
    lDevice.getSwapchainImagesKHR(m_swapchain, &imageCount, swapImages.data());

    m_imageViews.resize(imageCount);

    for (uint32_t i = 0; i < imageCount; ++i)
    {
        const vk::ImageViewCreateInfo createInfo = vk::ImageViewCreateInfo
        (
            vk::ImageViewCreateFlags(), 
            swapImages[i], 
            vk::ImageViewType::e2D, 
            m_surfaceFormat.format, 
            { vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity },
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)
        );

        if (lDevice.createImageView(&createInfo, nullptr, &m_imageViews[i]) != vk::Result::eSuccess)
        {
            printf("Failed to create Swapchain Image View \n");

            assert(0);
        }
    }
    TRACE("Created Vulkan SwapImages");

    m_framebuffers.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        const vk::ImageView attachments[] =
        {
            m_imageViews[i]
        };

        const vk::FramebufferCreateInfo framebufferInfo = vk::FramebufferCreateInfo
        (
            vk::FramebufferCreateFlags(),
            m_renderPass,
            1,
            attachments,
            (uint32_t)m_size.x,
            (uint32_t)m_size.y,
            1
        );

        if (lDevice.createFramebuffer(&framebufferInfo, nullptr, &m_framebuffers[i]) != vk::Result::eSuccess)
        {
            printf("Failed to create Swapchain Framebuffer");

            assert(0);
        }
    }
}
void VulkanSwapchain::Destroy()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    TRACE("Destroying ImageViews");
    for (const vk::ImageView& imageView : m_imageViews)
    {
        device.destroyImageView(imageView);
    }

    TRACE("Destroying Framebuffers")
    for (const vk::Framebuffer& framebuffer : m_framebuffers)
    {
        device.destroyFramebuffer(framebuffer);
    }
    
    TRACE("Destroying Swapchain");
    device.destroySwapchainKHR(m_swapchain);
}

VulkanSwapchain::VulkanSwapchain(VulkanRenderEngineBackend* a_engine, const glm::ivec2& a_size)
{
    m_engine = a_engine;

    const vk::Device device = m_engine->GetLogicalDevice();

    Init(a_size);
    
    const vk::AttachmentDescription colorAttachment = vk::AttachmentDescription
    (
        vk::AttachmentDescriptionFlags(),
        m_surfaceFormat.format,
        vk::SampleCountFlagBits::e1,
        vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore,
        vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::ePresentSrcKHR
    );

    constexpr vk::AttachmentReference ColorAttachmentRef = vk::AttachmentReference
    (
        0,
        vk::ImageLayout::eColorAttachmentOptimal
    );
    const vk::SubpassDescription subpass = vk::SubpassDescription
    (
        vk::SubpassDescriptionFlags(),
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        1,
        &ColorAttachmentRef
    );

    constexpr vk::SubpassDependency Dependency = vk::SubpassDependency
    (
        VK_SUBPASS_EXTERNAL,
        0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::PipelineStageFlagBits::eColorAttachmentOutput,
        vk::AccessFlags(),
        vk::AccessFlagBits::eColorAttachmentWrite
    );

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        vk::RenderPassCreateFlags(),
        1,
        &colorAttachment,
        1,
        &subpass,
        1,
        &Dependency
    );

    if (device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass) != vk::Result::eSuccess)
    {
        printf("Failed to create Swapchain Renderpass \n");

        assert(0);
    }
    TRACE("Created Vulkan Swapchain Renderpass");
}
VulkanSwapchain::~VulkanSwapchain()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    TRACE("Destroying RenderPass");
    device.destroyRenderPass(m_renderPass);

    Destroy();
}

SwapChainSupportInfo VulkanSwapchain::QuerySwapChainSupport(const vk::PhysicalDevice& a_device, const vk::SurfaceKHR& a_surface)
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

void VulkanSwapchain::Rebuild(const glm::ivec2& a_size)
{
    Destroy();
    Init(a_size);
}