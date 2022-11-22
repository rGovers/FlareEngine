#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "Rendering/RenderProgram.h"
#include "Rendering/Vulkan/VulkanConstants.h"

class CameraBuffer;
class ObjectManager;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanRenderPass;
class VulkanUniformBuffer;

class VulkanPipeline
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_camBufferAddr;
    RenderProgram              m_program;
    
    vk::Pipeline               m_pipeline;
    vk::PipelineLayout         m_layout;
    vk::DescriptorPool         m_descriptorPool;
    vk::DescriptorSetLayout    m_desciptorLayout;
    vk::DescriptorSet          m_descriptorSets[VulkanMaxFlightFrames];

    VulkanUniformBuffer*       m_cameraUniformBuffer = nullptr;
    ShaderBufferInput          m_cameraBufferInput;

protected:

public:
    VulkanPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, uint32_t a_camBufferAddr, const RenderProgram& a_program);
    ~VulkanPipeline();

    inline vk::Pipeline GetPipeline() const
    {
        return m_pipeline;
    }

    void UpdateCameraBuffer(uint32_t a_index, const glm::vec2& a_screenSize, const CameraBuffer& a_buffer, ObjectManager* a_objectManager) const;
    
    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};