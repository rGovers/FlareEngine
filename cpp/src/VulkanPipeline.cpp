#include "Rendering/Vulkan/VulkanPipeline.h"

#include "ObjectManager.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "Trace.h"

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
        else 
        {
            printf("Failed to find vertex shader \n");

            assert(0);
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
        else
        {
            printf("Failed to find pixel shader \n");

            assert(0);
        }
    }

    return stages;
}

constexpr static vk::ShaderStageFlags GetShaderStage(e_ShaderSlot a_slot) 
{
    switch (a_slot)
    {
    case ShaderSlot_Vertex:
    {
        return vk::ShaderStageFlagBits::eVertex;
    }
    case ShaderSlot_Pixel:
    {
        return vk::ShaderStageFlagBits::eFragment;
    }
    case ShaderSlot_All:
    {
        return vk::ShaderStageFlagBits::eAllGraphics;
    }
    }

    return vk::ShaderStageFlags();
}
constexpr static uint32_t GetBufferSize(e_ShaderBufferType a_type)
{
    switch (a_type)
    {
    case ShaderBufferType_Camera:
    {
        return sizeof(CameraShaderBuffer);
    }
    case ShaderBufferType_Model:
    {
        return sizeof(ModelShaderBuffer);
    }
    }
    
    return 0;
}

constexpr static void GetLayoutInfo(const RenderProgram& a_program, std::vector<vk::PushConstantRange>& a_pushConstants, std::vector<vk::DescriptorSetLayoutBinding>& a_ubos)
{
    for (uint16_t i = 0; i < a_program.ShaderBufferInputCount; ++i)
    {
        const ShaderBufferInput& input = a_program.ShaderBufferInputs[i];

        switch (input.BufferType)
        {
        case ShaderBufferType_Model:
        {
            a_pushConstants.push_back(vk::PushConstantRange
            (
                GetShaderStage(input.ShaderSlot),
                0,
                GetBufferSize(input.BufferType)
            ));

            break;
        }
        default:
        {
            a_ubos.push_back(vk::DescriptorSetLayoutBinding
            (
                input.Slot,
                vk::DescriptorType::eUniformBuffer,
                1,
                GetShaderStage(input.ShaderSlot)
            ));

            break;
        }
        }
    }
}

constexpr static vk::Format GetFormat(const VertexInputAttrib& a_attrib) 
{
    switch (a_attrib.Type)
    {
    case VertexType_Float:
    {
        switch (a_attrib.Count)
        {
        case 1:
        {
            return vk::Format::eR32Sfloat;
        }
        case 2:
        {
            return vk::Format::eR32G32Sfloat;
        }
        case 3:
        {
            return vk::Format::eR32G32B32Sfloat;
        }
        case 4:
        {
            return vk::Format::eR32G32B32A32Sfloat;
        }
        }

        break;
    }
    case VertexType_Int:
    {
        switch (a_attrib.Count)
        {
        case 1:
        {
            return vk::Format::eR32Sint;
        }
        case 2:
        {
            return vk::Format::eR32G32Sint;
        }
        case 3:
        {
            return vk::Format::eR32G32B32Sint;
        }
        case 4:
        {
            return vk::Format::eR32G32B32A32Sint;
        }
        }

        break;
    }
    case VertexType_UInt:
    {
        switch (a_attrib.Count)
        {
        case 1:
        {
            return vk::Format::eR32Uint;
        }
        case 2:
        {
            return vk::Format::eR32G32Uint;
        }
        case 3:
        {
            return vk::Format::eR32G32B32Uint;
        }
        case 4:
        {
            return vk::Format::eR32G32B32A32Uint;
        }
        }

        break;
    }
    }

    return vk::Format::eUndefined;
}

VulkanPipeline::VulkanPipeline(VulkanRenderEngineBackend* a_engine, const VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, uint32_t a_camBufferAddr, const RenderProgram& a_program)
{
    TRACE("Creating Vulkan Pipeline");
    m_engine = a_engine;

    m_camBufferAddr = a_camBufferAddr;
    m_program = a_program;

    const vk::Device device = m_engine->GetLogicalDevice();

    for (uint16_t i = 0; i < m_program.ShaderBufferInputCount; ++i)
    {
        switch (m_program.ShaderBufferInputs[i].BufferType)
        {
        case ShaderBufferType_Camera:
        {
            if (m_cameraUniformBuffer == nullptr)
            {
                m_cameraUniformBuffer = new VulkanUniformBuffer(m_engine, sizeof(CameraShaderBuffer));
                m_cameraBufferInput = m_program.ShaderBufferInputs[i];
            }

            break;
        }
        }
    }

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

    const vk::VertexInputBindingDescription bindingDescription = vk::VertexInputBindingDescription
    (
        0,
        a_program.VertexStride,
        vk::VertexInputRate::eVertex
    );

    std::vector<vk::VertexInputAttributeDescription> attributeDescription = std::vector<vk::VertexInputAttributeDescription>(a_program.VertexInputCount);
    for (uint16_t i = 0; i < a_program.VertexInputCount; ++i)
    {
        const VertexInputAttrib& attrib = a_program.VertexAttribs[i];
        attributeDescription[i].binding = 0;
        attributeDescription[i].location = attrib.Location;
        attributeDescription[i].offset = attrib.Offset;
        attributeDescription[i].format = GetFormat(attrib);
    }

    const vk::PipelineVertexInputStateCreateInfo vertexInputInfo = vk::PipelineVertexInputStateCreateInfo
    (
        vk::PipelineVertexInputStateCreateFlags(),
        1,
        &bindingDescription,
        (uint32_t)a_program.VertexInputCount,
        attributeDescription.data()
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

    std::vector<vk::PushConstantRange> pushConstants;
    std::vector<vk::DescriptorSetLayoutBinding> ubos;
    GetLayoutInfo(m_program, pushConstants, ubos);

    TRACE("Creating Pipeline Descriptor Layout");
    const vk::DescriptorSetLayoutCreateInfo descriptorLayout = vk::DescriptorSetLayoutCreateInfo
    (
        vk::DescriptorSetLayoutCreateFlags(),
        (uint32_t)ubos.size(),
        ubos.data()
    );
    vk::resultCheck(device.createDescriptorSetLayout(&descriptorLayout, nullptr, &m_desciptorLayout), "Failed to create Vulkan Descriptor Layout");

    TRACE("Creating Pipeline Descriptor Pool");
    constexpr vk::DescriptorPoolSize PoolSize = vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, VulkanMaxFlightFrames);
    const vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo
    (
        vk::DescriptorPoolCreateFlags(), 
        VulkanMaxFlightFrames, 
        1, 
        &PoolSize
    );
    vk::resultCheck(device.createDescriptorPool(&poolInfo, nullptr, &m_descriptorPool), "Failed to create Vulkan Descriptor Pool");

    TRACE("Creating Pipeline Descriptor Sets");
    vk::DescriptorSetLayout layouts[VulkanMaxFlightFrames];
    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        layouts[i] = m_desciptorLayout;
    }

    const vk::DescriptorSetAllocateInfo descriptorSetAllocInfo = vk::DescriptorSetAllocateInfo
    (
        m_descriptorPool,
        VulkanMaxFlightFrames,
        layouts
    );

    vk::resultCheck(device.allocateDescriptorSets(&descriptorSetAllocInfo, m_descriptorSets), "Failed to create Vulkan Descriptor Sets");

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        vk::PipelineLayoutCreateFlags(),
        1,
        &m_desciptorLayout,
        (uint32_t)pushConstants.size(),
        pushConstants.data()
    );

    TRACE("Creating Pipeline Layout");
    vk::resultCheck(device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_layout), "Failed to create Vulkan Pipeline Layout");
    
    const std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = GetStageInfo(a_program, a_gEngine);

    const vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo
    (
        vk::PipelineCreateFlags(),
        (uint32_t)shaderStages.size(),
        shaderStages.data(),
        &vertexInputInfo,
        &InputAssembly,
        nullptr,
        &viewportState,
        &Rasterizer,
        &Multisampling,
        nullptr,
        &colorBlending,
        &dynamicState,
        m_layout,
        a_renderPass
    );

    TRACE("Creating Pipeline");
    vk::resultCheck(device.createGraphicsPipelines(nullptr, 1, &pipelineInfo, nullptr, &m_pipeline), "Failed to create Vulkan Pipeline");
}
VulkanPipeline::~VulkanPipeline()
{
    const vk::Device device = m_engine->GetLogicalDevice();
    
    TRACE("Destroying Pipeline");

    device.destroyDescriptorPool(m_descriptorPool);
    device.destroyDescriptorSetLayout(m_desciptorLayout);
    device.destroyPipeline(m_pipeline);
    device.destroyPipelineLayout(m_layout);

    if (m_cameraUniformBuffer != nullptr)
    {
        delete m_cameraUniformBuffer;
        m_cameraUniformBuffer = nullptr;
    }
}

void VulkanPipeline::UpdateCameraBuffer(uint32_t a_index, const glm::vec2& a_screenSize, const CameraBuffer& a_buffer, const ObjectManager* a_objectManager)
{
    const vk::Device device = m_engine->GetLogicalDevice();

    if (m_cameraUniformBuffer != nullptr)
    {
        const TransformBuffer camTrans = a_objectManager->GetTransformBuffer(a_buffer.TransformAddr);

        CameraShaderBuffer buffer;
        buffer.InvView = camTrans.ToGlobalMat4(a_objectManager);
        buffer.View = glm::inverse(buffer.InvView);
        buffer.Proj = a_buffer.ToProjection(a_screenSize);
        buffer.InvProj = glm::inverse(buffer.Proj);
        buffer.ViewProj = buffer.Proj * buffer.View;

        m_cameraUniformBuffer->SetData(a_index, (char*)&buffer);

        const vk::DescriptorBufferInfo bufferInfo = vk::DescriptorBufferInfo
        (
            m_cameraUniformBuffer->GetBuffer(a_index),
            0,
            sizeof(CameraShaderBuffer)
        );

        const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
        (
            m_descriptorSets[a_index],
            m_cameraBufferInput.Slot,
            0,
            1,
            vk::DescriptorType::eUniformBuffer,
            nullptr,
            &bufferInfo
        );

        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    }
}
void VulkanPipeline::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 0, 1, &m_descriptorSets[a_index], 0, nullptr);

    a_commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline);
}