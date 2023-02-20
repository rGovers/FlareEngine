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
class VulkanRenderCommand;
class VulkanRenderEngineBackend;
class VulkanRenderTexture;
class VulkanSwapchain;
class VulkanVertexShader;

#include "DataTypes/TArray.h"
#include "DataTypes/TStatic.h"
#include "Rendering/CameraBuffer.h"
#include "Rendering/Light.h"
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
    VulkanSwapchain*                              m_swapchain;

    RuntimeFunction*                              m_preShadowFunc;
    RuntimeFunction*                              m_postShadowFunc;
    RuntimeFunction*                              m_preRenderFunc;
    RuntimeFunction*                              m_postRenderFunc;
    RuntimeFunction*                              m_preLightFunc;
    RuntimeFunction*                              m_postLightFunc;
    RuntimeFunction*                              m_postProcessFunc;

    VulkanRenderEngineBackend*                    m_vulkanEngine;

    std::mutex                                    m_pipeLock;
    std::unordered_map<uint64_t, VulkanPipeline*> m_pipelines;

    TStatic<VulkanRenderCommand>                  m_renderCommands;

    std::queue<uint32_t>                          m_freeShaderSlots;
    TArray<RenderProgram>                         m_shaderPrograms;
     
    TArray<VulkanVertexShader*>                   m_vertexShaders;
    TArray<VulkanPixelShader*>                    m_pixelShaders;
     
    TArray<VulkanModel*>                          m_models;
    TArray<VulkanRenderTexture*>                  m_renderTextures;

    TArray<MeshRenderBuffer>                      m_renderBuffers;
    TArray<MaterialRenderStack>                   m_renderStacks;

    TArray<DirectionalLightBuffer>                m_directionalLights;

    TArray<CameraBuffer>                          m_cameraBuffers;

    vk::CommandPool                               m_commandPool;
    
    vk::CommandBuffer DrawCamera(uint32_t a_camIndex);
    
protected:

public:
    VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine);
    ~VulkanGraphicsEngine();

    inline void SetSwapchain(VulkanSwapchain* a_swapchaing)
    {
        m_swapchain = a_swapchaing;
    }

    std::vector<vk::CommandBuffer> Update();

    VulkanVertexShader* GetVertexShader(uint32_t a_addr);
    VulkanPixelShader* GetPixelShader(uint32_t a_addr);

    VulkanPipeline* GetPipeline(uint32_t a_renderTexture, uint32_t a_pipeline);
    
    VulkanRenderTexture* GetRenderTexture(uint32_t a_addr);

    inline vk::CommandPool GetCommandPool() const
    {
        return m_commandPool;
    }
};