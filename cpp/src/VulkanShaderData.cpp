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

static PFN_vkCmdPushDescriptorSetKHR PushDescriptorSetKHRFunc = nullptr;

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
    case ShaderBufferType_DirectionalLightBuffer:
    {
        return sizeof(DirectionalLightShaderBuffer);
    }
    case ShaderBufferType_PointLightBuffer:
    {
        return sizeof(PointLightShaderBuffer);
    }
    case ShaderBufferType_SpotLightBuffer:
    {
        return sizeof(SpotLightShaderBuffer);
    }
    }
    
    return 0;
}
constexpr static vk::DescriptorType GetDescriptorType(e_ShaderBufferType a_bufferType)
{
    switch (a_bufferType)
    {
    case ShaderBufferType_Texture:
    case ShaderBufferType_PushTexture:
    {
        return vk::DescriptorType::eCombinedImageSampler;
    }
    }

    return vk::DescriptorType::eUniformBuffer;
}

constexpr static void GetLayoutInfo(const RenderProgram& a_program, std::vector<vk::PushConstantRange>& a_pushConstants, std::vector<vk::DescriptorSetLayoutBinding>& a_bindings, std::vector<vk::DescriptorSetLayoutBinding>& a_pushBindings)
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
        case ShaderBufferType_CameraBuffer:
        case ShaderBufferType_DirectionalLightBuffer:
        case ShaderBufferType_PointLightBuffer:
        case ShaderBufferType_SpotLightBuffer:
        case ShaderBufferType_PushTexture:
        {
            a_pushBindings.push_back(vk::DescriptorSetLayoutBinding
            (
                input.Slot,
                GetDescriptorType(input.BufferType),
                1,
                GetShaderStage(input.ShaderSlot)
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

    if (PushDescriptorSetKHRFunc == nullptr)
    {
        TRACE("Loading vkCmdPushDescriptorSetKHR Func");
        PushDescriptorSetKHRFunc = (PFN_vkCmdPushDescriptorSetKHR)vkGetInstanceProcAddr(m_engine->GetInstance(), "vkCmdPushDescriptorSetKHR");

        FLARE_ASSERT_R(PushDescriptorSetKHRFunc != nullptr);
    }

    m_programAddr = a_programAddr;

    m_staticDesciptorLayout = nullptr;
    m_pushDescriptorLayout = nullptr;
    m_descriptorSet = nullptr;

    TRACE("Creating Shader Data");
    const vk::Device device = m_engine->GetLogicalDevice();
    const RenderProgram program = m_gEngine->GetRenderProgram(m_programAddr);

    for (uint16_t i = 0; i < program.ShaderBufferInputCount; ++i)
    {
        switch (program.ShaderBufferInputs[i].BufferType)
        {
        case ShaderBufferType_CameraBuffer:
        {
            m_cameraBufferInput = program.ShaderBufferInputs[i];

            break;
        }
        case ShaderBufferType_ModelBuffer:
        {
            m_transformBufferInput = program.ShaderBufferInputs[i];

            break;
        }
        case ShaderBufferType_DirectionalLightBuffer:
        {
            m_directionalLightBufferInput = program.ShaderBufferInputs[i];

            break;
        }
        case ShaderBufferType_PointLightBuffer:
        {
            m_pointLightBufferInput = program.ShaderBufferInputs[i];

            break;
        }
        case ShaderBufferType_SpotLightBuffer:
        {
            m_spotLightBufferInput = program.ShaderBufferInputs[i];

            break;
        }
        }
    }

    std::vector<vk::PushConstantRange> pushConstants;
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    std::vector<vk::DescriptorSetLayoutBinding> pushBindings;
    GetLayoutInfo(program, pushConstants, bindings, pushBindings);

    std::vector<vk::DescriptorSetLayout> layouts;

    if (!pushBindings.empty())
    {
        TRACE("Creating Pipeline Push Descriptor Layout");
        const vk::DescriptorSetLayoutCreateInfo pushDescriptorLayout = vk::DescriptorSetLayoutCreateInfo
        (
            vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR,
            (uint32_t)pushBindings.size(),
            pushBindings.data()
        );

        FLARE_ASSERT_MSG_R(device.createDescriptorSetLayout(&pushDescriptorLayout, nullptr, &m_pushDescriptorLayout) == vk::Result::eSuccess, "Failed to create Push Descriptor Layout");

        layouts.emplace_back(m_pushDescriptorLayout);
    }

    if (!bindings.empty())
    {
        const uint32_t bindingCount = (uint32_t)bindings.size();

        TRACE("Creating Pipeline Descriptor Layout");
        const vk::DescriptorSetLayoutCreateInfo staticDescriptorLayout = vk::DescriptorSetLayoutCreateInfo
        (
            { },
            bindingCount,
            bindings.data()
        );
        FLARE_ASSERT_MSG_R(device.createDescriptorSetLayout(&staticDescriptorLayout, nullptr, &m_staticDesciptorLayout) == vk::Result::eSuccess, "Failed to create Static Descriptor Layout");

        TRACE("Creating Pipeline Descriptor Pool");
        std::vector<vk::DescriptorPoolSize> poolSizes = std::vector<vk::DescriptorPoolSize>(bindingCount);
        for (uint32_t i = 0; i < bindingCount; ++i)
        {
            poolSizes[i].type = bindings[i].descriptorType;
            poolSizes[i].descriptorCount = 1;
        }

        const vk::DescriptorPoolCreateInfo poolInfo = vk::DescriptorPoolCreateInfo
        (
            { }, 
            1, 
            bindingCount, 
            poolSizes.data()
        );
        FLARE_ASSERT_MSG_R(device.createDescriptorPool(&poolInfo, nullptr, &m_descriptorPool) == vk::Result::eSuccess, "Failed to create Descriptor Pool");

        TRACE("Creating Pipeline Descriptor Set");
        const vk::DescriptorSetAllocateInfo descriptorSetAllocInfo = vk::DescriptorSetAllocateInfo
        (
            m_descriptorPool,
            1,
            &m_staticDesciptorLayout
        );
        FLARE_ASSERT_MSG_R(device.allocateDescriptorSets(&descriptorSetAllocInfo, &m_descriptorSet) == vk::Result::eSuccess, "Failed to create Descriptor Sets");

        layouts.emplace_back(m_staticDesciptorLayout);
    }

    const vk::PipelineLayoutCreateInfo pipelineLayoutInfo = vk::PipelineLayoutCreateInfo
    (
        { },
        (uint32_t)layouts.size(),
        layouts.data(),
        (uint32_t)pushConstants.size(),
        pushConstants.data()
    );

    TRACE("Creating Pipeline Layout");
    FLARE_ASSERT_MSG_R(device.createPipelineLayout(&pipelineLayoutInfo, nullptr, &m_layout) == vk::Result::eSuccess, "Failed to create Pipeline Layout");
}
VulkanShaderData::~VulkanShaderData()
{
    TRACE("Destroying Shader Data");
    const vk::Device device = m_engine->GetLogicalDevice();

    if (m_staticDesciptorLayout != vk::DescriptorSetLayout(nullptr))
    {
        device.destroyDescriptorSetLayout(m_staticDesciptorLayout);
        device.destroyDescriptorPool(m_descriptorPool);
    }

    if (m_pushDescriptorLayout != vk::DescriptorSetLayout(nullptr))
    {
        device.destroyDescriptorSetLayout(m_pushDescriptorLayout);
    }

    device.destroyPipelineLayout(m_layout);
}

void VulkanShaderData::SetTexture(uint32_t a_slot, const TextureSampler& a_sampler) const
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
    case TextureMode_RenderTextureDepth:
    {
        const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(a_sampler.Addr);

        imageInfo.imageView = renderTexture->GetDepthImageView();
        imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

        break;
    }
    default:
    {
        FLARE_ASSERT_MSG(0, "SetTexture invalid texture mode");

        return;
    }
    }

    TRACE("Setting material render texture");
    const vk::WriteDescriptorSet descriptorWrite = vk::WriteDescriptorSet
    (
        m_descriptorSet,
        a_slot,
        0,
        1,
        vk::DescriptorType::eCombinedImageSampler,
        &imageInfo
    );

    device.updateDescriptorSets(1, &descriptorWrite, 0, nullptr);
}

void VulkanShaderData::PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSampler& a_sampler) const
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const VulkanTextureSampler* vSampler = (VulkanTextureSampler*)a_sampler.Data;
    FLARE_ASSERT(vSampler != nullptr);

    VkDescriptorImageInfo imageInfo;
    imageInfo.sampler = vSampler->GetSampler();

    switch (a_sampler.TextureMode)
    {
    case TextureMode_RenderTexture:
    {
        const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(a_sampler.Addr);

        imageInfo.imageView = renderTexture->GetImageView(a_sampler.TSlot);
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        break;
    }
    case TextureMode_RenderTextureDepth:
    {
        const VulkanRenderTexture* renderTexture = m_gEngine->GetRenderTexture(a_sampler.Addr);

        imageInfo.imageView = renderTexture->GetDepthImageView();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        break;
    }
    default:
    {
        FLARE_ASSERT_MSG(0, "SetTexture invalid texture mode");

        return;
    }
    }

    VkWriteDescriptorSet descriptorWrite = { };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = 0;
    descriptorWrite.dstBinding = a_slot;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.pImageInfo = &imageInfo;

    PushDescriptorSetKHRFunc((VkCommandBuffer)a_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)m_layout, 0, 1, &descriptorWrite);
}
void VulkanShaderData::PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, VulkanUniformBuffer* a_buffer, uint32_t a_index) const
{
    VkDescriptorBufferInfo bufferInfo = { };
    bufferInfo.buffer = a_buffer->GetBuffer(a_index);
    bufferInfo.offset = 0;
    bufferInfo.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet descriptorWrite = { };
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = 0;
    descriptorWrite.dstBinding = a_slot;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptorWrite.pBufferInfo = &bufferInfo;

    PushDescriptorSetKHRFunc((VkCommandBuffer)a_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipelineLayout)m_layout, 0, 1, &descriptorWrite);
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
    // TODO: Update
    if (m_descriptorSet != vk::DescriptorSet(nullptr))
    {
        a_commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_layout, 0, 1, &m_descriptorSet, 0, nullptr);
    }
}