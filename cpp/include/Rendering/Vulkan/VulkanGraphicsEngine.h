#pragma once

#include <queue>
#include <string_view>
#include <unordered_map>
#include <vector>

class RuntimeManager;
class VulkanPipeline;
class VulkanPixelShader;
class VulkanRenderEngineBackend;
class VulkanSwapchain;
class VulkanVertexShader;

#include "Rendering/CameraBuffer.h"
#include "Rendering/RenderProgram.h"

class VulkanGraphicsEngine
{
private:
    VulkanRenderEngineBackend*                    m_vulkanEngine;

    std::unordered_map<uint64_t, VulkanPipeline*> m_pipelines;

    std::queue<uint32_t>                          m_freeShaderSlots;
    std::vector<RenderProgram>                    m_shaderPrograms;
     
    std::vector<VulkanVertexShader*>              m_vertexShaders;
    std::vector<VulkanPixelShader*>               m_pixelShaders;
     
    std::queue<uint32_t>                          m_freeCamSlots;
    std::vector<CameraBuffer>                     m_cameraBuffers;

protected:

public:
    VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine);
    ~VulkanGraphicsEngine();

    void Update(VulkanSwapchain* a_swapChain);

    uint32_t GenerateVertexShaderAddr(const std::string_view& a_str);
    VulkanVertexShader* GetVertexShader(uint32_t a_addr) const;
    void DestroyVertexShader(uint32_t a_addr);

    uint32_t GeneratePixelShaderAddr(const std::string_view& a_str);
    VulkanPixelShader* GetPixelShader(uint32_t a_addr) const;
    void DestroyPixelShader(uint32_t a_addr);

    uint32_t GenerateShaderProgram(const RenderProgram& a_program);
    void DestroyShaderProgram(uint32_t a_addr);
    RenderProgram GetRenderProgram(uint32_t a_addr) const;
    void SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program);

    uint32_t GenerateCameraBuffer();
    void DestroyCameraBuffer(uint32_t a_addr);
    CameraBuffer GetCameraBuffer(uint32_t a_addr) const;
    void SetCameraBuffer(uint32_t a_add, const CameraBuffer& a_buffer);
};