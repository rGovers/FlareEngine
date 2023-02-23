#include "Rendering/Vulkan/VulkanRenderCommand.h"

#include "Logger.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

VulkanRenderCommand::VulkanRenderCommand(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_buffer)
{
    m_engine = a_engine;
    m_gEngine = a_gEngine;
    m_swapchain = a_swapchain;
    m_commandBuffer = a_buffer;

    m_renderTexAddr = -1;
    m_materialAddr = -1;
    m_flushed = true;
}
VulkanRenderCommand::~VulkanRenderCommand()
{
    
}

void VulkanRenderCommand::Flush()
{
    if (!m_flushed)
    {
        m_commandBuffer.endRenderPass();
    }

    m_flushed = true;

    m_renderTexAddr = -1;
}

VulkanRenderTexture* VulkanRenderCommand::GetRenderTexture() const
{
    return m_gEngine->GetRenderTexture(m_renderTexAddr);
}
VulkanPipeline* VulkanRenderCommand::GetPipeline() const
{
    if (m_materialAddr == -1)
    {
        return nullptr;
    }

    return m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
}

VulkanPipeline* VulkanRenderCommand::BindMaterial(uint32_t a_materialAddr)
{
    m_materialAddr = a_materialAddr;
    if (m_materialAddr == -1)
    {
        return nullptr;
    }

    VulkanPipeline* pipeline = m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
    pipeline->Bind(m_engine->GetCurrentFrame(), m_commandBuffer);

    return pipeline;
}

void VulkanRenderCommand::BindRenderTexture(uint32_t a_renderTexAddr)
{
    Flush();

    m_flushed = false;

    m_renderTexAddr = a_renderTexAddr;

    if (m_renderTexAddr == -1)
    {
        const glm::ivec2 renderSize = m_swapchain->GetSize();

        constexpr vk::ClearValue ClearColor = vk::ClearValue(vk::ClearColorValue(std::array{ 0.0f, 0.0f, 0.0f, 1.0f }));

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            m_swapchain->GetRenderPass(),
            m_swapchain->GetFramebuffer(m_engine->GetImageIndex()),
            vk::Rect2D({ 0, 0 }, { (uint32_t)renderSize.x, (uint32_t)renderSize.y }),
            1,
            &ClearColor
        );

        m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    }
    else
    {
        const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            renderTexture->GetRenderPass(),
            renderTexture->GetFramebuffer(),
            vk::Rect2D({ 0, 0 }, { renderTexture->GetWidth(), renderTexture->GetHeight() }),
            renderTexture->GetTotalTextureCount(),
            renderTexture->GetClearValues()
        );

        m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    }
}

void VulkanRenderCommand::Blit(const VulkanRenderTexture* a_src, const VulkanRenderTexture* a_dst)
{
    // TODO: Fix this temp fix for bliting
    // Probably better to copy or redraw when not flushed
    Flush();

    if (a_src == nullptr)
    {
        Logger::Error("FlareEngine: Cannot Blit Swapchain as Source");

        return;
    }

    const glm::ivec2 swapSize = m_swapchain->GetSize();

    vk::Image dstImage = m_swapchain->GetTexture();
    vk::Offset3D dstOffset = vk::Offset3D((int32_t)swapSize.x, (int32_t)swapSize.y, 1);
    vk::ImageLayout dstLayout = m_swapchain->GetImageLayout();
    if (a_dst != nullptr)
    {
        dstImage = a_dst->GetTexture(0);
        dstOffset = vk::Offset3D((int32_t)a_dst->GetWidth(), (int32_t)a_dst->GetHeight(), 1);
        dstLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    }

    const vk::Image srcImage = a_src->GetTexture(0);

    const vk::Offset3D srcOffset = vk::Offset3D((int32_t)a_src->GetWidth(), (int32_t)a_src->GetHeight(), 1);

    constexpr vk::Offset3D ZeroOffset;

    constexpr vk::ImageSubresourceLayers ImageSubResource = vk::ImageSubresourceLayers
    (
        vk::ImageAspectFlagBits::eColor, 
        0,
        0, 
        1
    );

    const vk::ImageBlit blitRegion = vk::ImageBlit
    (
        ImageSubResource, 
        { ZeroOffset, srcOffset }, 
        ImageSubResource, 
        { ZeroOffset, dstOffset }
    );

    constexpr vk::ImageSubresourceRange SubResourceRange = vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

    const vk::ImageMemoryBarrier srcMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eMemoryRead,
        vk::AccessFlagBits::eTransferRead,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        vk::ImageLayout::eTransferSrcOptimal,
        0,
        0,
        srcImage,
        SubResourceRange
    );
    const vk::ImageMemoryBarrier dstMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlags(),
        vk::AccessFlagBits::eTransferWrite,
        dstLayout,
        vk::ImageLayout::eTransferDstOptimal,
        0,
        0,
        dstImage,
        SubResourceRange
    );

    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &srcMemoryBarrier);
    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &dstMemoryBarrier);

    m_commandBuffer.blitImage(a_src->GetTexture(0), vk::ImageLayout::eTransferSrcOptimal, dstImage, vk::ImageLayout::eTransferDstOptimal, 1, &blitRegion, vk::Filter::eNearest);

    const vk::ImageMemoryBarrier srcFinalMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eTransferRead,
        vk::AccessFlagBits::eMemoryRead,
        vk::ImageLayout::eTransferSrcOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        0,
        0,
        srcImage,
        SubResourceRange
    );
    const vk::ImageMemoryBarrier dstFinalMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eMemoryRead,
        vk::ImageLayout::eTransferDstOptimal,
        dstLayout,
        0,
        0,
        dstImage,
        SubResourceRange
    );

    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &srcFinalMemoryBarrier);
    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &dstFinalMemoryBarrier);
}