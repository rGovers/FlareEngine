#include "Rendering/Vulkan/VulkanRenderTexture.h"

#include "Logger.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

static constexpr vk::Format GetFormat(bool a_hdr)
{
    if (a_hdr)
    {
        return vk::Format::eR16G16B16A16Sfloat;
    }

    return vk::Format::eR8G8B8A8Unorm;
}

VulkanRenderTexture::VulkanRenderTexture(VulkanRenderEngineBackend* a_engine, uint32_t a_textureCount, uint32_t a_width, uint32_t a_height, bool a_hdr)
{
    TRACE("Creating Render Texture");
    m_engine = a_engine;

    m_textureCount = a_textureCount;

    m_width = a_width;
    m_height = a_height;

    m_hdr = a_hdr;

    const VmaAllocator allocator = m_engine->GetAllocator();
    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::Format format = GetFormat(m_hdr);

    TRACE("Creating Attachments");
    std::vector<vk::AttachmentDescription> colorAttachments = std::vector<vk::AttachmentDescription>(m_textureCount);
    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        colorAttachments[i].format = format;
        colorAttachments[i].samples = vk::SampleCountFlagBits::e1;
        colorAttachments[i].loadOp = vk::AttachmentLoadOp::eClear;
        colorAttachments[i].storeOp = vk::AttachmentStoreOp::eStore;
        colorAttachments[i].stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
        colorAttachments[i].stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachments[i].initialLayout = vk::ImageLayout::eUndefined;
        colorAttachments[i].finalLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    }

    std::vector<vk::AttachmentReference> colorAttachmentRef = std::vector<vk::AttachmentReference>(m_textureCount);
    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        colorAttachmentRef[i].attachment = i;
        colorAttachmentRef[i].layout = vk::ImageLayout::eColorAttachmentOptimal;
    }

    const vk::SubpassDescription subpass = vk::SubpassDescription
    (
        { },
        vk::PipelineBindPoint::eGraphics,
        0,
        nullptr,
        m_textureCount,
        colorAttachmentRef.data()
    );

    vk::SubpassDependency dependencies[2];
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[0].dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[0].srcAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[0].dependencyFlags = vk::DependencyFlagBits::eByRegion;
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    dependencies[1].dstStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependencies[1].srcAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    dependencies[1].dstAccessMask = vk::AccessFlagBits::eMemoryRead;
    dependencies[1].dependencyFlags = vk::DependencyFlagBits::eByRegion;

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        { },
        (uint32_t)colorAttachments.size(),
        colorAttachments.data(),
        1,
        &subpass,
        2,
        dependencies
    );

    if (device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass) != vk::Result::eSuccess)
    {
        Logger::Error("Failed to create render texture render pass");

        assert(0);
    }    

    m_textures = new vk::Image[m_textureCount];
    m_textureViews = new vk::ImageView[m_textureCount];
    m_textureAllocations = new VmaAllocation[m_textureCount];
    m_clearValues = new vk::ClearValue[m_textureCount];
    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        m_clearValues[i] = vk::ClearValue({ 0.0f, 0.0f, 0.0f, 0.0f });
    }

    Init(m_width, m_height);
}
VulkanRenderTexture::~VulkanRenderTexture()
{
    TRACE("Destroying Render Texture");
    
    const vk::Device device = m_engine->GetLogicalDevice();

    Destroy();

    device.destroyRenderPass(m_renderPass);

    delete[] m_textures;
    delete[] m_textureViews;
    delete[] m_textureAllocations;
    delete[] m_clearValues;
}

void VulkanRenderTexture::Init(uint32_t a_width, uint32_t a_height)
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    const vk::Format format = GetFormat(m_hdr);

    m_width = a_width;
    m_height = a_height;

    TRACE("Creating Textures");
    VkImageCreateInfo textureCreateInfo = { };
    textureCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    textureCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    textureCreateInfo.format = (VkFormat)format;
    textureCreateInfo.extent.width = m_width;
    textureCreateInfo.extent.height = m_height;
    textureCreateInfo.extent.depth = 1;
    textureCreateInfo.mipLevels = 1;
    textureCreateInfo.arrayLayers = 1;
    // textureCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    textureCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    textureCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    textureCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    VmaAllocationCreateInfo allocInfo = { 0 };
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.memoryTypeBits = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    allocInfo.flags = 0;

    constexpr vk::ImageSubresourceRange SubresourceRange = vk::ImageSubresourceRange
    (
        vk::ImageAspectFlagBits::eColor,
        0,
        1,
        0,
        1
    );

    for (uint32_t i = 0; i < m_textureCount; ++i)
    {
        VkImage image;
        if (vmaCreateImage(allocator, &textureCreateInfo, &allocInfo, &image, &m_textureAllocations[i], nullptr) != VK_SUCCESS)
        {
            Logger::Error("Failed to create RenderTexture Image");

            assert(0);
        }
        m_textures[i] = image;

        const vk::ImageViewCreateInfo textureImageView = vk::ImageViewCreateInfo
        (
            { },
            m_textures[i],
            vk::ImageViewType::e2D,
            format,
            vk::ComponentMapping(),
            SubresourceRange
        );

        if (device.createImageView(&textureImageView, nullptr, &m_textureViews[i]) != vk::Result::eSuccess)
        {
            Logger::Error("Failed to create RenderTexture Image View");

            assert(0);
        }
    }

    TRACE("Creating Frame Buffer");
    const vk::FramebufferCreateInfo fbCreateInfo = vk::FramebufferCreateInfo
    (
        { },
        m_renderPass,
        m_textureCount,
        m_textureViews,
        m_width, 
        m_height,
        1
    );

    device.createFramebuffer(&fbCreateInfo, nullptr, &m_frameBuffer);
}
void VulkanRenderTexture::Destroy()
{
    const vk::Device device = m_engine->GetLogicalDevice();
    const VmaAllocator allocator = m_engine->GetAllocator();

    device.waitIdle();

    TRACE("Destroying Render Texture Textures");
    for (uint i = 0; i < m_textureCount; ++i)
    {
        vmaDestroyImage(allocator, m_textures[i], m_textureAllocations[i]);
        device.destroyImageView(m_textureViews[i]);
    }

    TRACE("Destroying Framebuffer");
    device.destroyFramebuffer(m_frameBuffer);
}

void VulkanRenderTexture::Resize(uint32_t a_width, uint32_t a_height)
{
    TRACE("Resizing Render Texture");
    Destroy();

    Init(a_width, a_height);
}