#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "Rendering/Vulkan/VulkanConstants.h"
#include "Rendering/ShaderBufferInput.h"

class ObjectManager;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanUniformBuffer;

struct TextureSampler;

class VulkanShaderData
{
private:
    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;

    uint32_t                   m_programAddr;

    vk::PipelineLayout         m_layout;

    vk::DescriptorSetLayout    m_staticDesciptorLayout;
    vk::DescriptorSetLayout    m_pushDescriptorLayout;

    vk::DescriptorPool         m_descriptorPool;
    vk::DescriptorSet          m_descriptorSet;

    ShaderBufferInput          m_cameraBufferInput;
    ShaderBufferInput          m_transformBufferInput;
    ShaderBufferInput          m_directionalLightBufferInput;
    ShaderBufferInput          m_pointLightBufferInput;

protected:

public:
    VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_programAddr);
    ~VulkanShaderData();

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }

    inline ShaderBufferInput GetCameraInput() const
    {
        return m_cameraBufferInput;
    }
    inline ShaderBufferInput GetDirectionalLightInput() const
    {
        return m_directionalLightBufferInput;
    }
    inline ShaderBufferInput GetPointLightInput() const
    {
        return m_pointLightBufferInput;
    }

    void SetTexture(uint32_t a_slot, const TextureSampler& a_sampler) const;

    void PushTexture(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, const TextureSampler& a_sampler) const;
    void PushUniformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_slot, VulkanUniformBuffer* a_buffer, uint32_t a_index) const;

    void UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_index, uint32_t a_transformAddr, ObjectManager* a_objectManager) const;

    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};