#include "Rendering/Vulkan/VulkanPipeline.h"

#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderPass.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"

static std::vector<vk::PipelineShaderStageCreateInfo> GetStageInfo(const RenderProgram& a_program, const VulkanGraphicsEngine* a_gEngine)
{
    std::vector<vk::PipelineShaderStageCreateInfo> stages;

    if (a_program.VertexShader != -1)
    {
        const VulkanShader* vertexShader = a_gEngine->GetVertexShader(a_program.VertexShader);
        if (vertexShader != nullptr)
        {
            stages.emplace_back(vk::PipelineShaderStageCreateInfo
            (
                vk::PipelineShaderStageCreateFlags(),
                vk::ShaderStageFlagBits::eVertex,
                vertexShader->GetShaderModule(),
                "main"
            ));
        }
    }

    if (a_program.PixelShader != -1)
    {
        const VulkanShader* pixelShader = a_gEngine->GetPixelShader(a_program.PixelShader);
        if (pixelShader != nullptr)
        {
            stages.emplace_back(vk::PipelineShaderStageCreateInfo
            (
                vk::PipelineShaderStageCreateFlags(),
                vk::ShaderStageFlagBits::eFragment,
                pixelShader->GetShaderModule(),
                "main"
            ));
        }
    }

    return stages;
}

VulkanPipeline::VulkanPipeline(VulkanRenderEngineBackend* a_engine, const VulkanGraphicsEngine* a_gEngine, const VulkanRenderPass* a_renderPass, uint32_t a_camBufferAddr, const RenderProgram& a_program)
{
    m_engine = a_engine;

    m_camBufferAddr = a_camBufferAddr;
    m_program = a_program;

    const std::vector<vk::DynamicState> dynamicStates = 
    {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    const vk::PipelineDynamicStateCreateInfo dynamicState = vk::PipelineDynamicStateCreateInfo
    (
        vk::PipelineDynamicStateCreateFlags(),
        (uint32_t)dynamicStates.size(),
        dynamicStates.data()
    );

    constexpr vk::PipelineVertexInputStateCreateInfo VertexInputInfo = vk::PipelineVertexInputStateCreateInfo
    (
        vk::PipelineVertexInputStateCreateFlags()
    );

    constexpr vk::PipelineInputAssemblyStateCreateInfo InputAssembly = vk::PipelineInputAssemblyStateCreateInfo
    (
        vk::PipelineInputAssemblyStateCreateFlags(),
        vk::PrimitiveTopology::eTriangleList,
        VK_FALSE
    );

    constexpr vk::Viewport Viewport = vk::Viewport(0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f);
    constexpr vk::Rect2D Scissor = vk::Rect2D({ 0, 0 }, { 1, 1 });

    const vk::PipelineViewportStateCreateInfo viewportState = vk::PipelineViewportStateCreateInfo
    (
        vk::PipelineViewportStateCreateFlags(),
        1,
        &Viewport,
        1,
        &Scissor
    );

    constexpr vk::PipelineRasterizationStateCreateInfo Rasterizer = vk::PipelineRasterizationStateCreateInfo
    (
        vk::PipelineRasterizationStateCreateFlags(),
        VK_FALSE,
        VK_FALSE,
        vk::PolygonMode::eFill,
        vk::CullModeFlagBits::eBack,
        vk::FrontFace::eClockwise,
        VK_FALSE,
        0.0f,
        0.0f,
        0.0f,
        1.0f
    );

    constexpr vk::PipelineMultisampleStateCreateInfo Multisampling = vk::PipelineMultisampleStateCreateInfo
    (
        vk::PipelineMultisampleStateCreateFlags(),
        vk::SampleCountFlagBits::e1
    );

    constexpr vk::PipelineColorBlendAttachmentState ColorBlendAttachment = vk::PipelineColorBlendAttachmentState
    (
        VK_FALSE,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::BlendFactor::eOne,
        vk::BlendFactor::eZero,
        vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    );

    const vk::PipelineColorBlendStateCreateInfo colorBlending = vk::PipelineColorBlendStateCreateInfo
    (
        vk::PipelineColorBlendStateCreateFlags(),
        VK_FALSE,
        vk::LogicOp::eCopy,
        1,
        &ColorBlendAttachment
    );

    constexpr vk::PipelineLayoutCreateInfo PipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        vk::PipelineLayoutCreateFlags()
    );

    const vk::Device device = m_engine->GetLogicalDevice();

    if (device.createPipelineLayout(&PipelineLayoutInfo, nullptr, &m_layout) != vk::Result::eSuccess)
    {
        printf("Failed to create Vulkan Pipeline Layout \n");

        assert(0);
    }
    
    const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = GetStageInfo(a_program, a_gEngine);

    const vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo
    (
        vk::PipelineCreateFlags(),
        (uint32_t)shaderStages.size(),
        shaderStages.data(),
        &VertexInputInfo,
        &InputAssembly,
        nullptr,
        &viewportState,
        &Rasterizer,
        &Multisampling,
        nullptr,
        &colorBlending,
        &dynamicState,
        m_layout,
        a_renderPass->GetRenderPass()
    );

    if (device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &m_pipeline) != vk::Result::eSuccess)
    {
        printf("Failed to create Vulkan Pipeline \n");

        assert(0);
    }
}
VulkanPipeline::~VulkanPipeline()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyPipeline(m_pipeline);
    device.destroyPipelineLayout(m_layout);
}