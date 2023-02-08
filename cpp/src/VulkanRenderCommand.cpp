#include "Rendering/Vulkan/VulkanRenderCommand.h"

#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

VulkanRenderCommand::VulkanRenderCommand(VulkanRenderEngineBackend* a_engine, VulkanSwapchain* a_swapchain, vk::CommandBuffer a_buffer)
{
    m_engine = a_engine;
    m_swapchain = a_swapchain;
    m_lastSemaphore = nullptr;
    m_commandBuffer = a_buffer;
}
VulkanRenderCommand::~VulkanRenderCommand()
{
    
}

void VulkanRenderCommand::Bind(VulkanRenderTexture* a_renderTexture, uint32_t a_renderTexAddr)
{
    m_renderTexAddr = a_renderTexAddr;

    if (a_renderTexture == nullptr)
    {
        constexpr vk::ClearValue ClearColor = vk::ClearValue(vk::ClearColorValue(std::array{ 0.0f, 0.0f, 0.0f, 1.0f }));

        m_renderSize = m_swapchain->GetSize();

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            m_swapchain->GetRenderPass(),
            m_swapchain->GetFramebuffer(m_engine->GetImageIndex()),
            vk::Rect2D({ 0, 0 }, { (uint32_t)m_renderSize.x, (uint32_t)m_renderSize.y }),
            1,
            &ClearColor
        );

        m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    }
    else
    {
        m_renderSize = glm::ivec2((int)a_renderTexture->GetWidth(), (int)a_renderTexture->GetHeight());

        const vk::RenderPassBeginInfo renderPassInfo = vk::RenderPassBeginInfo
        (
            a_renderTexture->GetRenderPass(),
            a_renderTexture->GetFramebuffer(),
            vk::Rect2D({ 0, 0 }, { (uint32_t)m_renderSize.x, (uint32_t)m_renderSize.y }),
            a_renderTexture->GetTextureCount(),
            a_renderTexture->GetClearValues()
        );

        m_commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    }
}