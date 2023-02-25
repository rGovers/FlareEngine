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

struct CameraBuffer;
struct TextureSampler;

class VulkanShaderData
{
private:
    VulkanRenderEngineBackend* m_engine;
    VulkanGraphicsEngine*      m_gEngine;

    uint32_t                   m_programAddr;

    vk::PipelineLayout         m_layout;

    vk::DescriptorPool         m_descriptorPool;
    vk::DescriptorSetLayout    m_desciptorLayout;
    vk::DescriptorSet          m_descriptorSets[VulkanMaxFlightFrames];

    VulkanUniformBuffer*       m_cameraUniformBuffer = nullptr;
    ShaderBufferInput          m_cameraBufferInput;

    ShaderBufferInput          m_transformBufferInput;

protected:

public:
    VulkanShaderData(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, uint32_t a_programAddr);
    ~VulkanShaderData();

    inline vk::PipelineLayout GetLayout() const
    {
        return m_layout;
    }

    void SetTexture(uint32_t a_slot, const TextureSampler& a_sampler) const;

    void UpdateCameraBuffer(uint32_t a_index, const glm::vec2& a_screenSize, const CameraBuffer& a_buffer, ObjectManager* a_objectManager) const;
    void UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_index, uint32_t a_transformAddr, ObjectManager* a_objectManager) const;

    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};