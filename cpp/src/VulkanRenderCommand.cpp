#include "Rendering/Vulkan/VulkanRenderCommand.h"

#include "FlareAssert.h"
#include "Logger.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
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
        VulkanRenderTexture* renderTexture = GetRenderTexture();
        if (renderTexture != nullptr)
        {
            renderTexture->SetShaderMode(true);
        }
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
    const bool bind = m_materialAddr != a_materialAddr;

    m_materialAddr = a_materialAddr;
    if (m_materialAddr == -1)
    {
        return nullptr;
    }

    VulkanPipeline* pipeline = m_gEngine->GetPipeline(m_renderTexAddr, m_materialAddr);
    if (bind)
    {
        pipeline->Bind(m_engine->GetCurrentFlightFrame(), m_commandBuffer);
    }

    return pipeline;
}

void VulkanRenderCommand::PushTexture(uint32_t a_slot, const TextureSampler& a_sampler) const
{
    FLARE_ASSERT_MSG_R(m_materialAddr != -1, "PushTexture Material not bound");

    const RenderProgram program = m_gEngine->GetRenderProgram(m_materialAddr);
    VulkanShaderData* data = (VulkanShaderData*)program.Data;

    data->PushTexture(m_commandBuffer, a_slot, a_sampler);
}

void VulkanRenderCommand::BindRenderTexture(uint32_t a_renderTexAddr)
{
    Flush();

    m_flushed = false;

    m_renderTexAddr = a_renderTexAddr;

    if (m_renderTexAddr == -1)
    {
        const glm::ivec2 renderSize = m_swapchain->GetSize();

        constexpr vk::ClearValue ClearColor = vk::ClearValue(vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f));

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
        VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(m_renderTexAddr);
        renderTexture->SetShaderMode(false);

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
        dstLayout = a_dst->GetImageLayout();
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

    const vk::ImageLayout srcLayout = a_src->GetImageLayout();

    const vk::ImageMemoryBarrier srcMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlags(),
        vk::AccessFlagBits::eTransferRead,
        srcLayout,
        vk::ImageLayout::eTransferSrcOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        srcImage,
        SubResourceRange
    );
    const vk::ImageMemoryBarrier dstMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlags(),
        vk::AccessFlagBits::eTransferWrite,
        dstLayout,
        vk::ImageLayout::eTransferDstOptimal,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
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
        srcLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        srcImage,
        SubResourceRange
    );
    const vk::ImageMemoryBarrier dstFinalMemoryBarrier = vk::ImageMemoryBarrier
    (
        vk::AccessFlagBits::eTransferWrite,
        vk::AccessFlagBits::eMemoryRead,
        vk::ImageLayout::eTransferDstOptimal,
        dstLayout,
        VK_QUEUE_FAMILY_IGNORED,
        VK_QUEUE_FAMILY_IGNORED,
        dstImage,
        SubResourceRange
    );

    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &srcFinalMemoryBarrier);
    m_commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &dstFinalMemoryBarrier);
}