#pragma once

#include <vulkan/vulkan.hpp>

#include "Rendering/RenderProgram.h"

class VulkanGraphicsEngine;
class VulkanRenderEngineBackend;
class VulkanRenderPass;

class VulkanPipeline
{
private:
    VulkanRenderEngineBackend* m_engine;

    uint32_t                   m_camBufferAddr;
    RenderProgram              m_program;

    vk::Pipeline               m_pipeline;
    vk::PipelineLayout         m_layout;

protected:

public:
    VulkanPipeline(VulkanRenderEngineBackend* a_engine, const VulkanGraphicsEngine* a_gEngine, const vk::RenderPass& a_renderPass, uint32_t a_camBufferAddr, const RenderProgram& a_program);
    ~VulkanPipeline();

    inline vk::Pipeline GetPipeline() const
    {
        return m_pipeline;
    }
};