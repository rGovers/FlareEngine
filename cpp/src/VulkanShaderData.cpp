#include "Rendering/Vulkan/VulkanShaderData.h"

#include "FlareAssert.h"
#include "ObjectManager.h"
#include "Rendering/ShaderBuffers.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanUniformBuffer.h"
#include "Trace.h"

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
    case ShaderBufferType_CameraBuffer:
    {
        return sizeof(CameraShaderBuffer);
    }
    case ShaderBufferType_ModelBuffer:
    {
        return sizeof(ModelShaderBuffer);
    }
    }
    
    return 0;
}
constexpr static vk::DescriptorType GetDescriptorType(e_ShaderBufferType a_bufferType)
{
    switch (a_bufferType)
    {
    case ShaderBufferType_Texture:
    {
        return vk::DescriptorType::eCombinedImageSampler;
    }
    }

    return vk::DescriptorType::eUniformBuffer;
}

constexpr static void GetLayoutInfo(const RenderProgram& a_program, std::vector<vk::PushConstantRange>& a_pushConstants, std::vector<vk::DescriptorSetLayoutBinding>& a_bindings)
{
    for (uint16_t i = 0; i < a_program.ShaderBufferInputCount; ++i)
    {
        const ShaderBufferInput& input = a_program.ShaderBufferInputs[i];

        switch (input.BufferType)
        {
        case ShaderBufferType_ModelBuffer:
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
            a_bindings.push_back(vk::DescriptorSetLayoutBinding
            (
                input.Slot,
                GetDescriptorType(input.BufferType),
                1,
                GetShaderStage(input.ShaderSlot)
            ));

            break;
        }
        }
    }
}

VulkanShaderData::VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_programAddr)
{
    m_engine = a_engine;
    m_gEngine = a_gEngine;

    m_programAddr = a_programAddr;

    TRACE("Creating Shader Data");
    const vk::Device device = m_engine->GetLogicalDevice();
    const RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);

    m_transformBufferInput.ShaderSlot = ShaderSlot_Null;

    for (uint16_t i = 0; i < program.ShaderBufferInputCount; ++i)
    {
        switch (program.ShaderBufferInputs[i].BufferType)
        {
        case ShaderBufferType_CameraBuffer:
        {
            if (m_cameraUniformBuffer == nullptr)
            {
                m_cameraUniformBuffer = new VulkanUniformBuffer(m_engine, sizeof(CameraShaderBuffer));
                m_cameraBufferInput = program.ShaderBufferInputs[i];
            }

            break;
        }
        case ShaderBufferType_ModelBuffer:
        {
            m_transformBufferInput = program.ShaderBufferInputs[i];

            break;
        }
        }
    }

    std::vector<vk::PushConstantRange> pushConstants;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    GetLayoutInfo(program, pushConstants, bindings);

    const uint32_t bindingCount = (uint32_t)bindings.size();

    TRACE("Creating Pipeline Descriptor Layout");
    const vk::DescriptorSetLayoutCreateInfo descriptorLayout = vk::DescriptorSetLayoutCreateInfo
    (
        { },
        bindingCount,
        bindings.data()
    );
    FLARE_ASSERT_MSG_R(device.createDescriptorSetLayout(&descriptorLayout, nullptr, &m_desciptorLayout) == vk::Result::eSuccess, "Failed to create Vulkan Descriptor Layout");

    TRACE("Creating Pipeline Descriptor Pool");
    std::vector<vk::DescriptorPoolSize> poolSizes = std::vector<vk::DescriptorPoolSize>(bindingCount);
    for (uint32_t i = 0; i < bindingCount; ++i)
    {
        poolSizes[i].type = bindings[i].descriptorType;
        poolSizes[i].descriptorCount = VulkanMaxFlightFrames;
    }
    
    const vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo
    (
        { }, 
        VulkanMaxFlightFrames, 
        bindingCount, 
        poolSizes.data()
    );
    FLARE_ASSERT_MSG_R(device.createDescriptorPool(&poolInfo, nullptr, &m_descriptorPool) == vk::Result::eSuccess, "Failed to create Vulkan Descriptor Pool");

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
    FLARE_ASSERT_MSG_R(device.allocateDescriptorSets(&descriptorSetAllocInfo, m_descriptorSets) == vk::Result::eSuccess, "Failed to create Vulkan Descriptor Sets");

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        vk::PipelineLayoutCreateFlags(),
        1,
        &m_desciptorLayout,
        (uint32_t)pushConstants.size(),
        pushConstants.data()
    );

    TRACE("Creating Pipeline Layout");
    FLARE_ASSERT_MSG_R(device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_layout) == vk::Result::eSuccess, "Failed to create Vulkan Pipeline Layout");
}
VulkanShaderData::~VulkanShaderData()
{
    TRACE("Destroying Shader Data");
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyDescriptorPool(m_descriptorPool);
    device.destroyDescriptorSetLayout(m_desciptorLayout);

    device.destroyPipelineLayout(m_layout);

    if (m_cameraUniformBuffer != nullptr)
    {
        delete m_cameraUniformBuffer;
        m_cameraUniformBuffer = nullptr;
    }
}

void VulkanShaderData::SetTexture(uint32_t a_index, const TextureSampler& a_sampler) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;

    FLARE_ASSERT(vSampler != nullptr);

    vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo(vSampler->GetSampler());

    switch (a_sampler.TextureMode)
    {
    case TextureMode_RenderTexture:
    {
        const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(a_sampler.Addr);

        imageInfo.imageView = renderTexture->GetImageView(a_sampler.TSlot);
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        break;
    }
    default:
    {
        FLARE_ASSERT_MSG(0, "SetTexture invalid texture mode");

        return;
    }
    }

    for (uint32_t i = 0; i < VulkanMaxFlightFrames; ++i)
    {
        const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
        (
            m_descriptorSets[i],
            a_index,
            0,
            1,
            vk::DescriptorType::eCombinedImageSampler,
            &imageInfo
        );

        device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
    }

    // const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
    // (
    //     m_descriptorSets[m_engine->GetImageIndex()],
    //     a_index,
    //     0,
    //     1,
    //     vk::DescriptorType::eCombinedImageSampler,
    //     &imageInfo
    // );

    // device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

void VulkanShaderData::UpdateCameraBuffer(uint32_t a_index, const glm::vec2& a_screenSize, const CameraBuffer& a_buffer, ObjectManager* a_objectManager) const
{
    if (m_cameraUniformBuffer != nullptr)
    {
        const vk::Device device = m_engine->GetLogicalDevice();

        CameraShaderBuffer buffer;
        buffer.InvView = a_objectManager->GetGlobalMatrix(a_index);
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
void VulkanShaderData::UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_index, uint32_t a_transformAddr, ObjectManager* a_objectManager) const
{
    if (m_transformBufferInput.ShaderSlot != ShaderSlot_Null)
    {
        ModelShaderBuffer buffer;
        buffer.Model = a_objectManager->GetGlobalMatrix(a_transformAddr);
        buffer.InvModel = glm::inverse(buffer.Model);

        a_commandBuffer.pushConstants(m_layout, GetShaderStage(m_transformBufferInput.ShaderSlot), 0, sizeof(ModelShaderBuffer), &buffer);
    }
}

void VulkanShaderData::Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const
{
    a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 0, 1, &m_descriptorSets[a_index], 0, nullptr);
}