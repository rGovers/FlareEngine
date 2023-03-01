#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"

#include "FlareAssert.h"
#include "Shaders/DirectionalLightPixel.h"
#include "Shaders/PointLightPixel.h"
#include "Shaders/QuadVertex.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static VulkanGraphicsEngineBindings* Engine = nullptr;

#define VULKANGRAPHICS_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

// The lazy part of me won against the part that wants to write clean code
// My apologies to the poor soul that has to decipher this definition
#define VULKANGRAPHICS_BINDING_FUNCTION_TABLE(F) \
    F(uint32_t, FlareEngine.Rendering, VertexShader, GenerateGLSLShader, { char* str = mono_string_to_utf8(a_string); const uint32_t ret = Engine->GenerateGLSLVertexShaderAddr(str); mono_free(str); return ret; }, MonoString* a_string) \
    F(uint32_t, FlareEngine.Rendering, VertexShader, GenerateFShader, { char* str = mono_string_to_utf8(a_string); const uint32_t ret = Engine->GenerateFVertexShaderAddr(str); mono_free(str); return ret; }, MonoString* a_string) \
    F(void, FlareEngine.Rendering, VertexShader, DestroyShader, { Engine->DestroyVertexShader(a_addr); }, uint32_t a_addr) \
    F(uint32_t, FlareEngine.Rendering, PixelShader, GenerateGLSLShader, { char* str = mono_string_to_utf8(a_string); const uint32_t ret = Engine->GenerateGLSLPixelShaderAddr(str); mono_free(str); return ret; }, MonoString* a_string) \
    F(uint32_t, FlareEngine.Rendering, PixelShader, GenerateFShader, { char* str = mono_string_to_utf8(a_string); const uint32_t ret = Engine->GenerateFPixelShaderAddr(str); mono_free(str); return ret; }, MonoString* a_string) \
    F(void, FlareEngine.Rendering, PixelShader, DestroyShader, { Engine->DestroyPixelShader(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, FlareEngine.Rendering, Material, GenerateInternalProgram, { return Engine->GenerateInternalShaderProgram(a_renderProgram); }, e_InternalRenderProgram a_renderProgram) \
    F(RenderProgram, FlareEngine.Rendering, Material, GetProgramBuffer, { return Engine->GetRenderProgram(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, Material, SetProgramBuffer, { Engine->SetRenderProgram(a_addr, a_program); }, uint32_t a_addr, RenderProgram a_program) \
    F(void, FlareEngine.Rendering, Material, SetTexture, { Engine->RenderProgramSetTexture(a_addr, a_shaderSlot, a_samplerAddr); }, uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr) \
    \
    F(uint32_t, FlareEngine.Rendering, Camera, GenerateBuffer, { return Engine->GenerateCameraBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, FlareEngine.Rendering, Camera, DestroyBuffer, { Engine->DestroyCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(CameraBuffer, FlareEngine.Rendering, Camera, GetBuffer, { return Engine->GetCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, Camera, SetBuffer, { Engine->SetCameraBuffer(a_addr, a_buffer); }, uint32_t a_addr, CameraBuffer a_buffer) \
    \
    F(uint32_t, FlareEngine.Rendering, MeshRenderer, GenerateBuffer, { return Engine->GenerateMeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr); }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr) \
    F(void, FlareEngine.Rendering, MeshRenderer, DestroyBuffer, { Engine->DestroyMeshRenderBuffer(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, MeshRenderer, GenerateRenderStack, { Engine->GenerateRenderStack(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, MeshRenderer, DestroyRenderStack, { Engine->DestroyRenderStack(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, FlareEngine.Rendering, TextureSampler, GenerateRenderTextureSampler, { return Engine->GenerateRenderTextureSampler(a_renderTexture, a_textureIndex, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_textureIndex, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, FlareEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSampler, { return Engine->GenerateRenderTextureDepthSampler(a_renderTexture, (e_TextureFilter)a_filter, (e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
    F(void, FlareEngine.Rendering, TextureSampler, DestroySampler, { Engine->DestroyTextureSampler(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, FlareEngine.Rendering, RenderTextureCmd, GenerateRenderTexture, { return Engine->GenerateRenderTexture(a_count, a_width, a_height, (bool)a_depthTexture, (bool)a_hdr); }, uint32_t a_count, uint32_t a_width, uint32_t a_height, uint32_t a_depthTexture, uint32_t a_hdr) \
    F(void, FlareEngine.Rendering, RenderTextureCmd, DestroyRenderTexture, { return Engine->DestroyRenderTexture(a_addr); }, uint32_t a_addr) \
    F(uint32_t, FlareEngine.Rendering, RenderTextureCmd, HasDepth, { return (uint32_t)Engine->RenderTextureHasDepth(a_addr); }, uint32_t a_addr) \
    F(uint32_t, FlareEngine.Rendering, RenderTextureCmd, GetWidth, { return Engine->GetRenderTextureWidth(a_addr); }, uint32_t a_addr) \
    F(uint32_t, FlareEngine.Rendering, RenderTextureCmd, GetHeight, { return Engine->GetRenderTextureHeight(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, RenderTextureCmd, Resize, { return Engine->ResizeRenderTexture(a_addr, a_width, a_height); }, uint32_t a_addr, uint32_t a_width, uint32_t a_height) \
    F(uint32_t, FlareEngine.Renddering, MultiRenderTexture, GetTextureCount, { return Engine->GetRenderTextureTextureCount(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, FlareEngine.Rendering.Lighting, DirectionalLight, GenerateBuffer, { return Engine->GenerateDirectionalLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, FlareEngine.Rendering.Lighting, DirectionalLight, DestroyBuffer, { Engine->DestroyDirectionalLightBuffer(a_addr); }, uint32_t a_addr) \
    F(DirectionalLightBuffer, FlareEngine.Rendering.Lighting, DirectionalLight, GetBuffer, { return Engine->GetDirectionalLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering.Lighting, DirectionalLight, SetBuffer, { Engine->SetDirectionalLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, DirectionalLightBuffer a_buffer) \
    \
    F(uint32_t, FlareEngine.Rendering.Lighting, PointLight, GenerateBuffer, { return Engine->GeneratePointLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, FlareEngine.Rendering.Lighting, PointLight, DestroyBuffer, { Engine->DestroyPointLightBuffer(a_addr); }, uint32_t a_addr) \
    F(PointLightBuffer, FlareEngine.Rendering.Lighting, PointLight, GetBuffer, { return Engine->GetPointLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering.Lighting, PointLight, SetBuffer, { Engine->SetPointLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, PointLightBuffer a_buffer) \
    \
    F(void, FlareEngine.Rendering, RenderCommand, BindMaterial, { Engine->BindMaterial(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, RenderCommand, PushTexture, { Engine->PushTexture(a_slot, a_samplerAddr); }, uint32_t a_slot, uint32_t a_samplerAddr) \
    F(void, FlareEngine.Rendering, RenderCommand, BindRenderTexture, { Engine->BindRenderTexture(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, RenderCommand, RTRTBlit, { Engine->BlitRTRT(a_srcAddr, a_dstAddr); }, uint32_t a_srcAddr, uint32_t a_dstAddr)

VULKANGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

// Gonna leave theses functions seperate as there is a bit to it
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(Material, GenerateProgram), uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_vertexInputAttribs, MonoArray* a_shaderInputs, uint32_t a_cullingMode, uint32_t a_primitiveMode)
{
    RenderProgram program;
    program.VertexShader = a_vertexShader;
    program.PixelShader = a_pixelShader;
    program.VertexStride = a_vertexStride;
    program.CullingMode = (e_CullMode)a_cullingMode;
    program.PrimitiveMode = (e_PrimitiveMode)a_primitiveMode;
    program.Flags = 0;

    // Need to recreate the array
    // Because it is a managed array may not be contiguous and is controlled by the GC 
    // Need a reliable lifetime and memory layout
    if (a_vertexInputAttribs != nullptr)
    {
        program.VertexInputCount = (uint16_t)mono_array_length(a_vertexInputAttribs);
        program.VertexAttribs = new VertexInputAttrib[program.VertexInputCount];

        for (uint16_t i = 0; i < program.VertexInputCount; ++i)
        {
            program.VertexAttribs[i] = mono_array_get(a_vertexInputAttribs, VertexInputAttrib, i);
        }
    }
    else
    {
        program.VertexInputCount = 0;
        program.VertexAttribs = nullptr;
    }
    
    if (a_shaderInputs != nullptr)
    {
        program.ShaderBufferInputCount = (uint16_t)mono_array_length(a_shaderInputs);
        program.ShaderBufferInputs = new ShaderBufferInput[program.ShaderBufferInputCount];

        for (uint16_t i = 0; i < program.ShaderBufferInputCount; ++i)
        {
            program.ShaderBufferInputs[i] = mono_array_get(a_shaderInputs, ShaderBufferInput, i);
        }
    }
    else
    {
        program.ShaderBufferInputCount = 0;
        program.ShaderBufferInputs = nullptr;
    }

    return Engine->GenerateShaderProgram(program);
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Material, DestroyProgram), uint32_t a_addr)
{
    const RenderProgram program = Engine->GetRenderProgram(a_addr);

    if (program.VertexAttribs != nullptr)
    {
        delete[] program.VertexAttribs;
    }
    if (program.ShaderBufferInputs != nullptr)
    {
        delete[] program.ShaderBufferInputs;
    }

    Engine->DestroyShaderProgram(a_addr);
}

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(Model, GenerateModel), MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride)
{
    const uint32_t vertexCount = (uint32_t)mono_array_length(a_vertices);
    const uint32_t indexCount = (uint32_t)mono_array_length(a_indices);

    const uint32_t vertexSize = vertexCount * a_vertexStride;

    char* vertices = new char[vertexSize];
    for (uint32_t i = 0; i < vertexSize; ++i)
    {
        vertices[i] = *mono_array_addr_with_size(a_vertices, 1, i);
    }

    uint32_t* indices = new uint32_t[indexCount];
    for (uint32_t i = 0; i < indexCount; ++i)
    {
        indices[i] = mono_array_get(a_indices, uint32_t, i);
    }

    const uint32_t addr = Engine->GenerateModel(vertices, vertexCount, indices, indexCount, a_vertexStride);

    delete[] vertices;
    delete[] indices;

    return addr;
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Model, DestroyModel), uint32_t a_addr)
{
    Engine->DestroyModel(a_addr);
}

VulkanGraphicsEngineBindings::VulkanGraphicsEngineBindings(RuntimeManager* a_runtime, VulkanGraphicsEngine* a_graphicsEngine)
{
    m_graphicsEngine = a_graphicsEngine;

    Engine = this;

    TRACE("Binding Vulkan functions to C#");
    VULKANGRAPHICS_BINDING_FUNCTION_TABLE(VULKANGRAPHICS_RUNTIME_ATTACH)

    a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine.Rendering, Material, GenerateProgram), (void*)RUNTIME_FUNCTION_NAME(Material, GenerateProgram));
    a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine.Rendering, Material, DestroyProgram), (void*)RUNTIME_FUNCTION_NAME(Material, DestroyProgram));

    a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine.Rendering, Model, GenerateModel), (void*)RUNTIME_FUNCTION_NAME(Model, GenerateModel));
    a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(FlareEngine.Rendering, Model, DestroyModel), (void*)RUNTIME_FUNCTION_NAME(Model, DestroyModel));
}
VulkanGraphicsEngineBindings::~VulkanGraphicsEngineBindings()
{

}

uint32_t VulkanGraphicsEngineBindings::GenerateFVertexShaderAddr(const std::string_view& a_str) const
{
    FLARE_ASSERT_MSG(!a_str.empty(), "GenerateFVertexShaderAddr empty string")

    VulkanVertexShader* shader = VulkanVertexShader::CreateFromFShader(m_graphicsEngine->m_vulkanEngine, a_str);

    const uint32_t size = m_graphicsEngine->m_vertexShaders.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_vertexShaders[i] == nullptr)
        {
            m_graphicsEngine->m_vertexShaders[i] = shader;
            
            return i;
        }
    }

    m_graphicsEngine->m_vertexShaders.Push(shader);

    return size;
}
uint32_t VulkanGraphicsEngineBindings::GenerateGLSLVertexShaderAddr(const std::string_view& a_str) const
{
    FLARE_ASSERT_MSG(!a_str.empty(), "GenerateGLSLVertexShaderAddr empty string")

    VulkanVertexShader* shader = VulkanVertexShader::CreateFromGLSL(m_graphicsEngine->m_vulkanEngine, a_str);

    const uint32_t size = m_graphicsEngine->m_vertexShaders.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_vertexShaders[i] == nullptr)
        {
            m_graphicsEngine->m_vertexShaders[i] = shader;
            
            return i;
        }
    }

    m_graphicsEngine->m_vertexShaders.Push(shader);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyVertexShader(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_vertexShaders.Size(), "DestroyVertexShader out of bounds")
    FLARE_ASSERT_MSG(m_graphicsEngine->m_vertexShaders[a_addr] != nullptr, "DestroyVertexShader already destroyed")

    delete m_graphicsEngine->m_vertexShaders[a_addr];
    m_graphicsEngine->m_vertexShaders[a_addr] = nullptr;
}

uint32_t VulkanGraphicsEngineBindings::GenerateFPixelShaderAddr(const std::string_view& a_str) const
{
    FLARE_ASSERT_MSG(!a_str.empty(), "GenerateFPixelShaderAddr empty string")

    VulkanPixelShader* shader = VulkanPixelShader::CreateFromFShader(m_graphicsEngine->m_vulkanEngine, a_str);

    const uint32_t size = (uint32_t)m_graphicsEngine->m_pixelShaders.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_pixelShaders[i] == nullptr)
        {
            m_graphicsEngine->m_pixelShaders[i] = shader;

            return i;
        }
    }

    m_graphicsEngine->m_pixelShaders.Push(shader);

    return size;
}
uint32_t VulkanGraphicsEngineBindings::GenerateGLSLPixelShaderAddr(const std::string_view& a_str) const
{
    FLARE_ASSERT_MSG(!a_str.empty(), "GenerateGLSLPixelShaderAddr empty string")

    VulkanPixelShader* shader = VulkanPixelShader::CreateFromGLSL(m_graphicsEngine->m_vulkanEngine, a_str);

    const uint32_t size = (uint32_t)m_graphicsEngine->m_pixelShaders.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_pixelShaders[i] == nullptr)
        {
            m_graphicsEngine->m_pixelShaders[i] = shader;

            return i;
        }
    }

    m_graphicsEngine->m_pixelShaders.Push(shader);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyPixelShader(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_pixelShaders.Size(), "DestroyPixelShader out of bounds")
    FLARE_ASSERT_MSG(m_graphicsEngine->m_pixelShaders[a_addr] != nullptr, "DestroyPixelShader already destroyed")

    delete m_graphicsEngine->m_pixelShaders[a_addr];
    m_graphicsEngine->m_pixelShaders[a_addr] = nullptr;
}

uint32_t VulkanGraphicsEngineBindings::GenerateInternalShaderProgram(e_InternalRenderProgram a_program) const
{
    RenderProgram program;
    program.VertexStride = 0;
    program.VertexInputCount = 0;
    program.VertexAttribs = nullptr;
    program.Flags |= 0b1 << RenderProgram::DestroyFlag;

    switch (a_program)
    {
    case InternalRenderProgram_DirectionalLight:
    {
        TRACE("Creating Directional Light Shader");
        program.VertexShader = GenerateFVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(DIRECTIONALLIGHTPIXEL);
        program.CullingMode = CullMode_None;
        program.PrimitiveMode = PrimitiveMode_TriangleStrip;

        constexpr uint32_t TextureCount = 5;
        constexpr uint32_t BufferCount = TextureCount + 2;

        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = ShaderBufferInput(i, ShaderBufferType_PushTexture, ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = ShaderBufferInput(TextureCount + 0, ShaderBufferType_DirectionalLightBuffer, ShaderSlot_Pixel);
        program.ShaderBufferInputs[TextureCount + 1] = ShaderBufferInput(TextureCount + 1, ShaderBufferType_CameraBuffer, ShaderSlot_Pixel);

        break;
    }
    case InternalRenderProgram_PointLight:
    {
        TRACE("Creating Point Light Shader");
        program.VertexShader = GenerateFVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(POINTLIGHTPIXEL);
        program.CullingMode = CullMode_None;
        program.PrimitiveMode = PrimitiveMode_TriangleStrip;

        constexpr uint32_t TextureCount = 5;
        constexpr uint32_t BufferCount = TextureCount + 2;
        
        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = ShaderBufferInput(i, ShaderBufferType_PushTexture, ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = ShaderBufferInput(TextureCount + 0, ShaderBufferType_PointLightBuffer, ShaderSlot_Pixel);
        program.ShaderBufferInputs[TextureCount + 1] = ShaderBufferInput(TextureCount + 1, ShaderBufferType_CameraBuffer, ShaderSlot_Pixel);

        break;
    }
    default:
    {
        FLARE_ASSERT_MSG(0, "Invalid Internal Render Program");
    }
    }

    return GenerateShaderProgram(program);
}
uint32_t VulkanGraphicsEngineBindings::GenerateShaderProgram(RenderProgram& a_program) const
{
    FLARE_ASSERT_MSG(a_program.PixelShader < m_graphicsEngine->m_pixelShaders.Size(), "GenerateShaderProgram PixelShader out of bounds")
    FLARE_ASSERT_MSG(a_program.VertexShader < m_graphicsEngine->m_vertexShaders.Size(), "GenerateShaderProgram VertexShader out of bounds")

    TRACE("Creating Shader Program");

    const uint32_t size = (uint32_t)m_graphicsEngine->m_shaderPrograms.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_shaderPrograms[i].Flags & 0b1 << RenderProgram::FreeFlag)
        {
            FLARE_ASSERT_MSG(m_graphicsEngine->m_shaderPrograms[i].Data == nullptr, "GenerateShaderProgram data not deleted");

            m_graphicsEngine->m_shaderPrograms[i] = a_program;
            m_graphicsEngine->m_shaderPrograms[i].Data = new VulkanShaderData(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine, i);

            return i;
        }
    }

    TRACE("Allocating Shader Program");

    m_graphicsEngine->m_shaderPrograms.Push(a_program);
    m_graphicsEngine->m_shaderPrograms[size].Data = new VulkanShaderData(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine, size);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyShaderProgram(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "DestroyShaderProgram out of bounds");

    RenderProgram& program = m_graphicsEngine->m_shaderPrograms[a_addr];
    if (program.Flags & 0b1 << RenderProgram::DestroyFlag)
    {
        DestroyVertexShader(program.VertexShader);
        DestroyPixelShader(program.PixelShader);
    }
    program.Flags = 0b1 << RenderProgram::FreeFlag;

    if (program.Data != nullptr)
    {
        delete (VulkanShaderData*)program.Data;
        program.Data = nullptr;
    }
}
void VulkanGraphicsEngineBindings::RenderProgramSetTexture(uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr)
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "RenderProgramSetTexture material out of bounds");
    FLARE_ASSERT_MSG(a_samplerAddr < m_graphicsEngine->m_textureSampler.Size(), "RenderProgramSetTexture sampler out of bounds");

    RenderProgram& program = m_graphicsEngine->m_shaderPrograms[a_addr];

    FLARE_ASSERT_MSG(program.Data != nullptr, "RenderProgramSetTexture invalid program");

    VulkanShaderData* data = (VulkanShaderData*)program.Data;
    data->SetTexture(a_shaderSlot, m_graphicsEngine->m_textureSampler[a_samplerAddr]);
}
RenderProgram VulkanGraphicsEngineBindings::GetRenderProgram(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "GetRenderProgram out of bounds")

    return m_graphicsEngine->m_shaderPrograms[a_addr];
}
void VulkanGraphicsEngineBindings::SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "SetRenderProgram out of bounds")

    m_graphicsEngine->m_shaderPrograms[a_addr] = a_program;
}

uint32_t VulkanGraphicsEngineBindings::GenerateCameraBuffer(uint32_t a_transformAddr) const
{
    FLARE_ASSERT_MSG(a_transformAddr != -1, "GenerateCameraBuffer invalid transform address")

    const CameraBuffer buff = CameraBuffer(a_transformAddr);

    TRACE("Getting Camera Buffer");
    const uint32_t size = m_graphicsEngine->m_cameraBuffers.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_cameraBuffers[i].TransformAddr == -1)
        {
            m_graphicsEngine->m_cameraBuffers[i] = buff;

            return i;
        }
    }

    TRACE("Allocating Camera Buffer");
    m_graphicsEngine->m_cameraBuffers.Push(buff);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyCameraBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "DestroyCameraBuffer out of bounds")
    m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr = -1;

    uint32_t val = m_graphicsEngine->m_cameraBuffers[m_graphicsEngine->m_cameraBuffers.Size() - 1].TransformAddr;
    while (val == -1)
    {
        TRACE("Destroying Camera Buffer");
        m_graphicsEngine->m_cameraBuffers.Pop();

        if (m_graphicsEngine->m_cameraBuffers.Empty())
        {
            break;
        }

        val = m_graphicsEngine->m_cameraBuffers[m_graphicsEngine->m_cameraBuffers.Size() - 1].TransformAddr;
    }
}
CameraBuffer VulkanGraphicsEngineBindings::GetCameraBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "GetCameraBuffer out of bounds")

    return m_graphicsEngine->m_cameraBuffers[a_addr];
}
void VulkanGraphicsEngineBindings::SetCameraBuffer(uint32_t a_addr, const CameraBuffer& a_buffer) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "SetCameraBuffer out of bounds")

    m_graphicsEngine->m_cameraBuffers[a_addr] = a_buffer;
}

uint32_t VulkanGraphicsEngineBindings::GenerateModel(const char* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride) const
{
    FLARE_ASSERT_MSG(a_vertices != nullptr, "GenerateModel vertices null")
    FLARE_ASSERT_MSG(a_vertexCount > 0, "GenerateModel no vertices")
    FLARE_ASSERT_MSG(a_indices != nullptr, "GenerateModel indices null")
    FLARE_ASSERT_MSG(a_indexCount > 0, "GenerateModel no indices")
    FLARE_ASSERT_MSG(a_vertexStride > 0, "GenerateModel vertex stride 0")

    VulkanModel* model = new VulkanModel(m_graphicsEngine->m_vulkanEngine, a_vertexCount, a_vertices, a_vertexStride, a_indexCount, a_indices);

    const uint32_t size = m_graphicsEngine->m_models.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_models[i] == nullptr)
        {
            m_graphicsEngine->m_models[i] = model;

            return i;
        }
    }

    TRACE("Allocating Model Buffer");
    m_graphicsEngine->m_models.Push(model);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyModel(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_models.Size(), "DestroyModel out of bounds")
    FLARE_ASSERT_MSG(m_graphicsEngine->m_models[a_addr] != nullptr, "DestroyModel already destroyed")

    delete m_graphicsEngine->m_models[a_addr];
    m_graphicsEngine->m_models[a_addr] = nullptr;
}

uint32_t VulkanGraphicsEngineBindings::GenerateMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr) const
{
    const MeshRenderBuffer buffer = MeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr);

    TRACE("Creating Render Buffer");
    const uint32_t size = (uint32_t)m_graphicsEngine->m_renderBuffers.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_renderBuffers[i].MaterialAddr == -1)
        {
            m_graphicsEngine->m_renderBuffers[i] = buffer;

            return i;
        }
    }

    TRACE("Allocating Render Buffer");
    m_graphicsEngine->m_renderBuffers.Push(buffer);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyMeshRenderBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderBuffers.Size(), "DestroyMeshRenderBuffer out of bounds");

    TRACE("Destroying Render Buffer");
    m_graphicsEngine->m_renderBuffers[a_addr].MaterialAddr = -1;
}
void VulkanGraphicsEngineBindings::GenerateRenderStack(uint32_t a_meshAddr) const
{
    FLARE_ASSERT_MSG(a_meshAddr < m_graphicsEngine->m_renderBuffers.Size(), "GenerateRenderStack out of bounds");

    TRACE("Pushing RenderStack");
    const MeshRenderBuffer& buffer = m_graphicsEngine->m_renderBuffers[a_meshAddr];

    std::mutex& lock = m_graphicsEngine->m_renderStacks.Lock();
    lock.lock();

    const uint32_t size = m_graphicsEngine->m_renderStacks.Size();
    MaterialRenderStack* renderStacks = m_graphicsEngine->m_renderStacks.Data();

    for (uint32_t i = 0; i < size; ++i)
    {
        if (renderStacks[i].Add(buffer))
        {
            lock.unlock();

            return;
        }
    }

    lock.unlock();

    TRACE("Allocating RenderStack");
    m_graphicsEngine->m_renderStacks.Push(buffer);
}
void VulkanGraphicsEngineBindings::DestroyRenderStack(uint32_t a_meshAddr) const
{
    FLARE_ASSERT_MSG(a_meshAddr < m_graphicsEngine->m_renderBuffers.Size(), "DestroyRenderStack out of bounds");

    TRACE("Removing RenderStack");
    const MeshRenderBuffer& buffer = m_graphicsEngine->m_renderBuffers[a_meshAddr];

    std::mutex& lock = m_graphicsEngine->m_renderStacks.Lock();
    lock.lock();

    const uint32_t size = m_graphicsEngine->m_renderStacks.Size();
    MaterialRenderStack* renderStacks = m_graphicsEngine->m_renderStacks.Data();

    for (uint32_t i = 0; i < size; ++i)
    {
        MaterialRenderStack& stack = renderStacks[i];
        if (stack.Remove(buffer))
        {
            lock.unlock();

            if (stack.Empty())
            {
                TRACE("Destroying RenderStack");
                m_graphicsEngine->m_renderStacks.Erase(i);
            }

            return;
        }
    }

    lock.unlock();
}

uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureSampler(uint32_t a_renderTexture, uint32_t a_textureIndex, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const
{ 
    FLARE_ASSERT_MSG(a_renderTexture < m_graphicsEngine->m_renderTextures.Size(), "GenerateRenderTextureSampler RenderTexture out of bounds");
    FLARE_ASSERT_MSG(m_graphicsEngine->m_renderTextures[a_renderTexture] != nullptr, "GenerateRenderTextureSampler RenderTexture destroyed");
    FLARE_ASSERT_MSG(a_textureIndex < m_graphicsEngine->m_renderTextures[a_renderTexture]->GetTextureCount(), "GenerateRenderTextureSampler texture index out of bounds");

    TextureSampler sampler;
    sampler.Addr = a_renderTexture;
    sampler.TextureMode = TextureMode_RenderTexture;
    sampler.TSlot = a_textureIndex;
    sampler.FilterMode = a_filter;
    sampler.AddressMode = a_addressMode;
    sampler.Data = new VulkanTextureSampler(m_graphicsEngine->m_vulkanEngine, sampler);

    const uint32_t size = m_graphicsEngine->m_textureSampler.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_textureSampler[i].TextureMode == TextureMode_Null)
        {
            FLARE_ASSERT_MSG(m_graphicsEngine->m_textureSampler[i].Data == nullptr, "GenerateRenderTextureSampler null sampler with data")

            m_graphicsEngine->m_textureSampler[i] = sampler;

            return i;
        }
    }

    m_graphicsEngine->m_textureSampler.Push(sampler);

    return size;
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureDepthSampler(uint32_t a_renderTexture, e_TextureFilter a_filter, e_TextureAddress a_addressMode) const
{
    FLARE_ASSERT_MSG(a_renderTexture < m_graphicsEngine->m_renderTextures.Size(), "GenerateRenderTextureDepthSampler out of bounds");
    FLARE_ASSERT_MSG(m_graphicsEngine->m_renderTextures[a_renderTexture] != nullptr, "GenerateRenderTextureDepthSampler RenderTexture destroyed");

    TextureSampler sampler;
    sampler.Addr = a_renderTexture;
    sampler.TextureMode = TextureMode_RenderTextureDepth;
    sampler.FilterMode = a_filter;
    sampler.AddressMode = a_addressMode;
    sampler.Data = new VulkanTextureSampler(m_graphicsEngine->m_vulkanEngine, sampler);

    const uint32_t size = m_graphicsEngine->m_textureSampler.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_textureSampler[i].TextureMode == TextureMode_Null)
        {
            FLARE_ASSERT_MSG(m_graphicsEngine->m_textureSampler[i].Data == nullptr, "GenerateRenderTextureDepthSampler null sampler with data")

            m_graphicsEngine->m_textureSampler[i] = sampler;

            return i;
        }
    }

    m_graphicsEngine->m_textureSampler.Push(sampler);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyTextureSampler(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_textureSampler.Size(), "DestroyTextureSampler out of bounds");
    
    m_graphicsEngine->m_textureSampler[a_addr].TextureMode = TextureMode_Null;
    if (m_graphicsEngine->m_textureSampler[a_addr].Data != nullptr)
    {
        delete (VulkanTextureSampler*)m_graphicsEngine->m_textureSampler[a_addr].Data;
        m_graphicsEngine->m_textureSampler[a_addr].Data = nullptr;
    }
}

uint32_t VulkanGraphicsEngineBindings::GenerateRenderTexture(uint32_t a_count, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr) const
{
    FLARE_ASSERT_MSG(a_count > 0, "GenerateRenderTexture no textures");
    FLARE_ASSERT_MSG(a_width > 0, "GenerateRenderTexture width 0");
    FLARE_ASSERT_MSG(a_height > 0, "GenerateRenderTexture height 0");

    VulkanRenderEngineBackend* engine = m_graphicsEngine->m_vulkanEngine;

    VulkanRenderTexture* texture = new VulkanRenderTexture(engine, a_count, a_width, a_height, a_depthTexture, a_hdr);

    const uint32_t size = m_graphicsEngine->m_renderTextures.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_renderTextures[i] == nullptr)
        {
            m_graphicsEngine->m_renderTextures[i] = texture;

            return i;
        }
    }

    TRACE("Allocating RenderTexture Buffer");
    m_graphicsEngine->m_renderTextures.Push(texture);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyRenderTexture(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "DestroyRenderTexture out of bounds");

    delete m_graphicsEngine->m_renderTextures[a_addr];

    m_graphicsEngine->m_renderTextures[a_addr] = nullptr;
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureTextureCount(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "GetRenderTextureCount out of bounds");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetTextureCount();
}
bool VulkanGraphicsEngineBindings::RenderTextureHasDepth(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "RenderTextureHasDepth out of bounds");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->HasDepthTexture();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureWidth(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "GetRenderTextureWidth out of bounds");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetWidth();
}
uint32_t VulkanGraphicsEngineBindings::GetRenderTextureHeight(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "GetRenderTextureHeight out of bounds");

    const VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    return texture->GetHeight();
}
void VulkanGraphicsEngineBindings::ResizeRenderTexture(uint32_t a_addr, uint32_t a_width, uint32_t a_height) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "ResizeRenderTexture out of bounds");
    FLARE_ASSERT_MSG(a_width > 0, "ResizeRenderTexture width 0")
    FLARE_ASSERT_MSG(a_height > 0, "ResizeRenderTexture height 0")

    VulkanRenderTexture* texture = m_graphicsEngine->m_renderTextures[a_addr];

    texture->Resize(a_width, a_height);
}

uint32_t VulkanGraphicsEngineBindings::GenerateDirectionalLightBuffer(uint32_t a_transformAddr) const
{
    const DirectionalLightBuffer buffer = DirectionalLightBuffer(a_transformAddr);

    FLARE_ASSERT_MSG(buffer.TransformAddr != -1, "GenerateDirectionalLightBuffer no transform");

    const uint32_t size = m_graphicsEngine->m_directionalLights.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_directionalLights[i].TransformAddr != -1)
        {
            m_graphicsEngine->m_directionalLights[i] = buffer;

            return i;
        }
    }

    TRACE("Allocating DirectionalLight Buffer");
    m_graphicsEngine->m_directionalLights.Push(buffer);

    return size;
}
void VulkanGraphicsEngineBindings::SetDirectionalLightBuffer(uint32_t a_addr, const DirectionalLightBuffer& a_buffer) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "SetDirectionalLightBuffer out of bounds");

    m_graphicsEngine->m_directionalLights[a_addr] = a_buffer;
}
DirectionalLightBuffer VulkanGraphicsEngineBindings::GetDirectionalLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "GetDirectionalLightBuffer out of bounds");

    return m_graphicsEngine->m_directionalLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroyDirectionalLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "DestroyDirectionalLightBuffer out of bounds");

    m_graphicsEngine->m_directionalLights[a_addr].TransformAddr = -1;
}

uint32_t VulkanGraphicsEngineBindings::GeneratePointLightBuffer(uint32_t a_transformAddr) const
{
    const PointLightBuffer buffer = PointLightBuffer(a_transformAddr);

    FLARE_ASSERT_MSG(buffer.TransformAddr != -1, "GeneratePointLightBuffer no transform");

    const uint32_t size = m_graphicsEngine->m_pointLights.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_pointLights[i].TransformAddr != -1)
        {
            m_graphicsEngine->m_pointLights[i] = buffer;

            return i;
        }
    }

    TRACE("Allocating PointLight Buffer");
    m_graphicsEngine->m_pointLights.Push(buffer);

    return size;
}
void VulkanGraphicsEngineBindings::SetPointLightBuffer(uint32_t a_addr, const PointLightBuffer& a_buffer) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "SetPointLightBuffer out of bounds");

    m_graphicsEngine->m_pointLights[a_addr] = a_buffer;
}
PointLightBuffer VulkanGraphicsEngineBindings::GetPointLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "GetPointLightBuffer out of bounds");

    return m_graphicsEngine->m_pointLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroyPointLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "DestroyPointLightBuffer out of bounds");

    m_graphicsEngine->m_pointLights[a_addr].TransformAddr = -1;
}

void VulkanGraphicsEngineBindings::BindMaterial(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "BindMaterial RenderCommand does not exist");
    if (a_addr != -1)
    {
        FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "BindMaterial out of bounds");
    }

    m_graphicsEngine->m_renderCommands->BindMaterial(a_addr);
}
void VulkanGraphicsEngineBindings::PushTexture(uint32_t a_slot, uint32_t a_samplerAddr) const
{
    FLARE_ASSERT_MSG_R(a_samplerAddr < m_graphicsEngine->m_textureSampler.Size(), "PushTexture sampler out of bounds");

    m_graphicsEngine->m_renderCommands->PushTexture(a_slot, m_graphicsEngine->m_textureSampler[a_samplerAddr]);
}
void VulkanGraphicsEngineBindings::BindRenderTexture(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "BindRenderTexture RenderCommand does not exist");

    VulkanRenderTexture* tex = nullptr;
    if (a_addr != -1)
    {
        FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "BindRenderTexture out of bounds");

        tex = m_graphicsEngine->m_renderTextures[a_addr];
    }

    m_graphicsEngine->m_renderCommands->BindRenderTexture(a_addr);
}
void VulkanGraphicsEngineBindings::BlitRTRT(uint32_t a_srcAddr, uint32_t a_dstAddr) const
{
    FLARE_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "BlitRTRT RenderCommand does not exist");
    
    VulkanRenderTexture* srcTex = nullptr;
    VulkanRenderTexture* dstTex = nullptr;

    if (a_srcAddr != -1)
    {
        FLARE_ASSERT_MSG(a_srcAddr < m_graphicsEngine->m_renderTextures.Size(), "BlitRTRT source out of bounds");

        srcTex = m_graphicsEngine->m_renderTextures[a_srcAddr];
    }
    if (a_dstAddr != -1)
    {
        FLARE_ASSERT_MSG(a_dstAddr < m_graphicsEngine->m_renderTextures.Size(), "BlitRTRT destination out of bounds");

        dstTex = m_graphicsEngine->m_renderTextures[a_dstAddr];
    }

    m_graphicsEngine->m_renderCommands->Blit(srcTex, dstTex);
}