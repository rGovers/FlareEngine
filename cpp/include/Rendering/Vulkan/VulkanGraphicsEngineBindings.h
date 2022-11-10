#pragma once

#include <string_view>

class RuntimeManager;
class VulkanGraphicsEngine;
class VulkanPixelShader;
class VulkanVertexShader;

#include "Rendering/CameraBuffer.h"
#include "Rendering/MeshRenderBuffer.h"
#include "Rendering/RenderProgram.h"

class VulkanGraphicsEngineBindings
{
private:
    VulkanGraphicsEngine* m_graphicsEngine;

protected:

public:
    VulkanGraphicsEngineBindings(RuntimeManager* a_runtime, VulkanGraphicsEngine* a_graphicsEngine);
    ~VulkanGraphicsEngineBindings();

    uint32_t GenerateVertexShaderAddr(const std::string_view& a_str);
    void DestroyVertexShader(uint32_t a_addr);

    uint32_t GeneratePixelShaderAddr(const std::string_view& a_str);
    void DestroyPixelShader(uint32_t a_addr);

    uint32_t GenerateShaderProgram(const RenderProgram& a_program);
    void DestroyShaderProgram(uint32_t a_addr);
    RenderProgram GetRenderProgram(uint32_t a_addr) const;
    void SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program);

    uint32_t GenerateCameraBuffer(uint32_t a_transformAddr);
    void DestroyCameraBuffer(uint32_t a_addr);
    CameraBuffer GetCameraBuffer(uint32_t a_addr) const;
    void SetCameraBuffer(uint32_t a_add, const CameraBuffer& a_buffer);

    uint32_t GenerateModel(const char* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride);
    void DestroyModel(uint32_t a_addr);

    uint32_t GenerateMeshRenderBuffer(const MeshRenderBuffer& a_renderBuffer);
    void DestroyMeshRenderBuffer(uint32_t a_addr);
    void GenerateRenderStack(uint32_t a_meshAddr);
    void DestroyRenderStack(uint32_t a_meshAddr);

    inline VulkanGraphicsEngine* GetGraphicsEngine() const
    {
        return m_graphicsEngine;
    }
};