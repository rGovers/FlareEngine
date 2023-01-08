#pragma once

#include <queue>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.hpp>

class RuntimeFunction;
class RuntimeManager;
class VulkanGraphicsEngineBindings;
class VulkanModel;
class VulkanPipeline;
class VulkanPixelShader;
class VulkanRenderEngineBackend;
class VulkanSwapchain;
class VulkanVertexShader;

#include "DataTypes/TArray.h"
#include "Rendering/CameraBuffer.h"
#include "Rendering/MaterialRenderStack.h"
#include "Rendering/MeshRenderBuffer.h"
#include "Rendering/RenderProgram.h"

class VulkanGraphicsEngine
{
private:
    friend class VulkanGraphicsEngineBindings;

    // This despite not being needed is used to fix a crash
    RuntimeManager*                               m_runtimeManager;
    VulkanGraphicsEngineBindings*                 m_runtimeBindings;

    RuntimeFunction*                              m_preShadowFunc;
    RuntimeFunction*                              m_postShadowFunc;
    RuntimeFunction*                              m_preRenderFunc;
    RuntimeFunction*                              m_postRenderFunc;
    RuntimeFunction*                              m_postProcessFunc;

    VulkanRenderEngineBackend*                    m_vulkanEngine;

    std::unordered_map<uint64_t, VulkanPipeline*> m_pipelines;

    std::queue<uint32_t>                          m_freeShaderSlots;
    TArray<RenderProgram>                         m_shaderPrograms;
     
    TArray<VulkanVertexShader*>                   m_vertexShaders;
    TArray<VulkanPixelShader*>                    m_pixelShaders;
     
    TArray<VulkanModel*>                          m_models;

    TArray<MeshRenderBuffer>                      m_renderBuffers;
    TArray<MaterialRenderStack>                   m_renderStacks;

    TArray<CameraBuffer>                          m_cameraBuffers;

    vk::CommandPool                               m_commandPool;

    vk::CommandBuffer DrawCamera(uint32_t a_camIndex, const VulkanSwapchain* a_swapchain);

protected:

public:
    VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine);
    ~VulkanGraphicsEngine();

    std::vector<vk::CommandBuffer> Update(const VulkanSwapchain* a_swapChain);

    VulkanVertexShader* GetVertexShader(uint32_t a_addr);
    VulkanPixelShader* GetPixelShader(uint32_t a_addr);

    inline vk::CommandPool GetCommandPool() const
    {
        return m_commandPool;
    }
};