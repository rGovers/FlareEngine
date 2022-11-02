#include "Rendering/Vulkan/VulkanRenderPass.h"

#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

VulkanRenderPass::VulkanRenderPass(VulkanRenderEngineBackend* a_engine, const VulkanSwapchain* a_swapchain)
{   
    m_engine = a_engine;

    const vk::AttachmentDescription colorAttachment = vk::AttachmentDescription
    (
        vk::AttachmentDescriptionFlags(),
        a_swapchain->GetSurfaceFormat().format,
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

    const vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo
    (
        vk::RenderPassCreateFlags(),
        1,
        &colorAttachment,
        1,
        &subpass
    );

    const vk::Device device = m_engine->GetLogicalDevice();
    if (device.createRenderPass(&renderPassInfo, nullptr, &m_renderPass) != vk::Result::eSuccess)
    {
        printf("Failed to create Swapchain Renderpass \n");

        assert(0);
    }
}
VulkanRenderPass::~VulkanRenderPass()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyRenderPass(m_renderPass);
}