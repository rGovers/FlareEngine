#pragma once

#include <string_view>

class RuntimeManager;
class VulkanGraphicsEngine;
class VulkanPixelShader;
class VulkanVertexShader;

#include "Rendering/CameraBuffer.h"
#include "Rendering/Light.h"
#include "Rendering/MeshRenderBuffer.h"
#include "Rendering/RenderProgram.h"
#include "Rendering/TextureSampler.h"

class VulkanGraphicsEngineBindings
{
private:
    VulkanGraphicsEngine* m_graphicsEngine;

protected:

public:
    VulkanGraphicsEngineBindings(RuntimeManager* a_runtime, VulkanGraphicsEngine* a_graphicsEngine);
    ~VulkanGraphicsEngineBindings();

    uint32_t GenerateFVertexShaderAddr(const std::string_view& a_str) const;
    uint32_t GenerateGLSLVertexShaderAddr(const std::string_view& a_str) const;
    void DestroyVertexShader(uint32_t a_addr) const;

    uint32_t GenerateFPixelShaderAddr(const std::string_view& a_str) const;
    uint32_t GenerateGLSLPixelShaderAddr(const std::string_view& a_str) const;
    void DestroyPixelShader(uint32_t a_addr) const;

    uint32_t GenerateInternalShaderProgram(e_InternalRenderProgram a_program) const;
    uint32_t GenerateShaderProgram(RenderProgram& a_program) const;
    void DestroyShaderProgram(uint32_t a_addr) const;
    void RenderProgramSetTexture(uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr);
    RenderProgram GetRenderProgram(uint32_t a_addr) const;
    void SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program) const;

    uint32_t GenerateCameraBuffer(uint32_t a_transformAddr) const;
    void DestroyCameraBuffer(uint32_t a_addr) const;
    CameraBuffer GetCameraBuffer(uint32_t a_addr) const;
    void SetCameraBuffer(uint32_t a_add, const CameraBuffer& a_buffer) const;

    uint32_t GenerateModel(const char* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride) const;
    void DestroyModel(uint32_t a_addr) const;

    uint32_t GenerateMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr) const;
    void DestroyMeshRenderBuffer(uint32_t a_addr) const;
    void GenerateRenderStack(uint32_t a_meshAddr) const;
    void DestroyRenderStack(uint32_t a_meshAddr) const;

    uint32_t GenerateRenderTextureSampler(uint32_t a_renderTexture, uint32_t a_textureIndex, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const;
    uint32_t GenerateRenderTextureDepthSampler(uint32_t a_renderTexture, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const;
    void DestroyTextureSampler(uint32_t a_addr) const;

    uint32_t GenerateRenderTexture(uint32_t a_count, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr) const;
    void DestroyRenderTexture(uint32_t a_addr) const;
    uint32_t GetRenderTextureTextureCount(uint32_t a_addr) const;
    bool RenderTextureHasDepth(uint32_t a_addr) const;
    uint32_t GetRenderTextureWidth(uint32_t a_addr) const;
    uint32_t GetRenderTextureHeight(uint32_t a_addr) const;
    void ResizeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const;

    uint32_t GenerateDirectionalLightBuffer(uint32_t a_transformAddr) const;
    void SetDirectionalLightBuffer(uint32_t a_addr, const DirectionalLightBuffer& a_buffer) const;
    DirectionalLightBuffer GetDirectionalLightBuffer(uint32_t a_addr) const;
    void DestroyDirectionalLightBuffer(uint32_t a_addr) const;

    void BindMaterial(uint32_t a_addr) const;
    void BindRenderTexture(uint32_t a_addr) const;
    void BlitRTRT(uint32_t a_srcAddr, uint32_t a_dstAddr) const;
};