#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <vulkan/vulkan.hpp>

#include "Rendering/RenderProgram.h"
#include "Rendering/Vulkan/VulkanConstants.h"

class ObjectManager;
class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanRenderPass;
class VulkanUniformBuffer;

struct CameraBuffer;
struct TransformBuffer;

class VulkanPipeline
{
private:
    VulkanRenderEngineBackend* m_engine;

    RenderProgram              m_program;
    
    vk::Pipeline               m_pipeline;
    vk::PipelineLayout         m_layout;
    vk::DescriptorPool         m_descriptorPool;
    vk::DescriptorSetLayout    m_desciptorLayout;
    vk::DescriptorSet          m_descriptorSets[VulkanMaxFlightFrames];

    VulkanUniformBuffer*       m_cameraUniformBuffer = nullptr;
    ShaderBufferInput          m_cameraBufferInput;

    ShaderBufferInput          m_transformBufferInput;
protected:

public:
    VulkanPipeline(VulkanRenderEngineBackend* a_engine, VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, const RenderProgram& a_program);
    ~VulkanPipeline();

    inline vk::Pipeline GetPipeline() const
    {
        return m_pipeline;
    }

    void UpdateCameraBuffer(uint32_t a_index, const glm::vec2& a_screenSize, const CameraBuffer& a_buffer, ObjectManager* a_objectManager) const;
    void UpdateTransformBuffer(vk::CommandBuffer a_commandBuffer, uint32_t a_index, TransformBuffer& a_buffer, ObjectManager* a_objectManager) const;

    void Bind(uint32_t a_index, vk::CommandBuffer a_commandBuffer) const;
};