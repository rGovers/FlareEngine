#pragma once

#include <queue>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

class RuntimeManager;
class VulkanGraphicsEngineBindings;
class VulkanModel;
class VulkanPipeline;
class VulkanPixelShader;
class VulkanRenderEngineBackend;
class VulkanSwapchain;
class VulkanVertexShader;

#include "Rendering/CameraBuffer.h"
#include "Rendering/MaterialRenderStack.h"
#include "Rendering/MeshRenderBuffer.h"
#include "Rendering/RenderProgram.h"

class VulkanGraphicsEngine
{
private:
    friend class VulkanGraphicsEngineBindings;

    VulkanGraphicsEngineBindings*                 m_runtimeBindings;
    VulkanRenderEngineBackend*                    m_vulkanEngine;

    std::unordered_map<uint64_t, VulkanPipeline*> m_pipelines;

    std::queue<uint32_t>                          m_freeShaderSlots;
    std::vector<RenderProgram>                    m_shaderPrograms;
     
    std::vector<VulkanVertexShader*>              m_vertexShaders;
    std::vector<VulkanPixelShader*>               m_pixelShaders;
     
    std::vector<VulkanModel*>                     m_models;

    std::vector<MeshRenderBuffer>                 m_renderBuffers;
    std::vector<MaterialRenderStack>              m_renderStacks;

    std::vector<CameraBuffer>                     m_cameraBuffers;

    vk::CommandPool                               m_commandPool;

protected:

public:
    VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine);
    ~VulkanGraphicsEngine();

    std::vector<vk::CommandBuffer> Update(const VulkanSwapchain* a_swapChain);

    VulkanVertexShader* GetVertexShader(uint32_t a_addr) const;
    VulkanPixelShader* GetPixelShader(uint32_t a_addr) const;

    inline vk::CommandPool GetCommandPool() const
    {
        return m_commandPool;
    }
};