#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"

#include <fstream>
#include <sstream>
#include <stb_image.h>

#include "Flare/ColladaLoader.h"
#include "Flare/FlareAssert.h"
#include "Flare/OBJLoader.h"
#include "ObjectManager.h"
#include "Shaders/DirectionalLightPixel.h"
#include "Shaders/PointLightPixel.h"
#include "Shaders/PostPixel.h"
#include "Shaders/QuadVertex.h"
#include "Shaders/SpotLightPixel.h"
#include "Rendering/RenderEngine.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderCommand.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanRenderTexture.h"
#include "Rendering/Vulkan/VulkanShaderData.h"
#include "Rendering/Vulkan/VulkanTexture.h"
#include "Rendering/Vulkan/VulkanTextureSampler.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

static VulkanGraphicsEngineBindings* Engine = nullptr;

#define VULKANGRAPHICS_RUNTIME_ATTACH(ret, namespace, klass, name, code, ...) a_runtime->BindFunction(RUNTIME_FUNCTION_STRING(namespace, klass, name), (void*)RUNTIME_FUNCTION_NAME(klass, name));

// The lazy part of me won against the part that wants to write clean code
// My apologies to the poor soul that has to decipher this definition
#define VULKANGRAPHICS_BINDING_FUNCTION_TABLE(F) \
    F(void, FlareEngine.Rendering, VertexShader, DestroyShader, { Engine->DestroyVertexShader(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, PixelShader, DestroyShader, { Engine->DestroyPixelShader(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, FlareEngine.Rendering, Material, GenerateInternalProgram, { return Engine->GenerateInternalShaderProgram(a_renderProgram); }, FlareBase::e_InternalRenderProgram a_renderProgram) \
    F(FlareBase::RenderProgram, FlareEngine.Rendering, Material, GetProgramBuffer, { return Engine->GetRenderProgram(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, Material, SetProgramBuffer, { Engine->SetRenderProgram(a_addr, a_program); }, uint32_t a_addr, FlareBase::RenderProgram a_program) \
    F(void, FlareEngine.Rendering, Material, SetTexture, { Engine->RenderProgramSetTexture(a_addr, a_shaderSlot, a_samplerAddr); }, uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr) \
    \
    F(uint32_t, FlareEngine.Rendering, Camera, GenerateBuffer, { return Engine->GenerateCameraBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, FlareEngine.Rendering, Camera, DestroyBuffer, { Engine->DestroyCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(CameraBuffer, FlareEngine.Rendering, Camera, GetBuffer, { return Engine->GetCameraBuffer(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, Camera, SetBuffer, { Engine->SetCameraBuffer(a_addr, a_buffer); }, uint32_t a_addr, CameraBuffer a_buffer) \
    F(glm::vec3, FlareEngine.Rendering, Camera, ScreenToWorld, { return Engine->CameraScreenToWorld(a_addr, a_screenPos, a_screenSize); }, uint32_t a_addr, glm::vec3 a_screenPos, glm::vec2 a_screenSize) \
    \
    F(uint32_t, FlareEngine.Rendering, MeshRenderer, GenerateBuffer, { return Engine->GenerateMeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr); }, uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr) \
    F(void, FlareEngine.Rendering, MeshRenderer, DestroyBuffer, { Engine->DestroyMeshRenderBuffer(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, MeshRenderer, GenerateRenderStack, { Engine->GenerateRenderStack(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, MeshRenderer, DestroyRenderStack, { Engine->DestroyRenderStack(a_addr); }, uint32_t a_addr) \
    \
    F(void, FlareEngine.Rendering, Texture, DestroyTexture, { Engine->DestroyTexture(a_addr); }, uint32_t a_addr) \
    \
    F(uint32_t, FlareEngine.Rendering, TextureSampler, GenerateTextureSampler, { return Engine->GenerateTextureSampler(a_texture, (FlareBase::e_TextureFilter)a_filter, (FlareBase::e_TextureAddress)a_addressMode ); }, uint32_t a_texture, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, FlareEngine.Rendering, TextureSampler, GenerateRenderTextureSampler, { return Engine->GenerateRenderTextureSampler(a_renderTexture, a_textureIndex, (FlareBase::e_TextureFilter)a_filter, (FlareBase::e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_textureIndex, uint32_t a_filter, uint32_t a_addressMode) \
    F(uint32_t, FlareEngine.Rendering, TextureSampler, GenerateRenderTextureDepthSampler, { return Engine->GenerateRenderTextureDepthSampler(a_renderTexture, (FlareBase::e_TextureFilter)a_filter, (FlareBase::e_TextureAddress)a_addressMode); }, uint32_t a_renderTexture, uint32_t a_filter, uint32_t a_addressMode) \
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
    F(uint32_t, FlareEngine.Rendering.Lighting, SpotLight, GenerateBuffer, { return Engine->GenerateSpotLightBuffer(a_transformAddr); }, uint32_t a_transformAddr) \
    F(void, FlareEngine.Rendering.Lighting, SpotLight, DestroyBuffer, { Engine->DestroySpotLightBuffer(a_addr); }, uint32_t a_addr) \
    F(SpotLightBuffer, FlareEngine.Rendering.Lighting, SpotLight, GetBuffer, { return Engine->GetSpotLightBuffer(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering.Lighting, SpotLight, SetBuffer, { Engine->SetSpotLightBuffer(a_addr, a_buffer); }, uint32_t a_addr, SpotLightBuffer a_buffer) \
    \
    F(void, FlareEngine.Rendering, RenderCommand, BindMaterial, { Engine->BindMaterial(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, RenderCommand, PushTexture, { Engine->PushTexture(a_slot, a_samplerAddr); }, uint32_t a_slot, uint32_t a_samplerAddr) \
    F(void, FlareEngine.Rendering, RenderCommand, BindRenderTexture, { Engine->BindRenderTexture(a_addr); }, uint32_t a_addr) \
    F(void, FlareEngine.Rendering, RenderCommand, RTRTBlit, { Engine->BlitRTRT(a_srcAddr, a_dstAddr); }, uint32_t a_srcAddr, uint32_t a_dstAddr) \
    F(void, FlareEngine.Rendering, RenderCommand, DrawMaterial, { Engine->DrawMaterial(); }) 

VULKANGRAPHICS_BINDING_FUNCTION_TABLE(RUNTIME_FUNCTION_DEFINITION)

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(VertexShader, GenerateFromFile), MonoString* a_path)
{
    char* str = mono_string_to_utf8(a_path);

    const std::filesystem::path p = std::filesystem::path(str);

    mono_free(str);  

    if (p.extension() == ".fvert")
    {
        std::ifstream file = std::ifstream(p);
        if (file.good() && file.is_open())
        {
            std::stringstream ss;

            ss << file.rdbuf();

            file.close();

            return Engine->GenerateFVertexShaderAddr(ss.str());
        }
    }
    else if (p.extension() == ".vert")
    {
        std::ifstream file = std::ifstream(p);
        if (file.good() && file.is_open())
        {
            std::stringstream ss;

            ss << file.rdbuf();

            file.close();

            return Engine->GenerateGLSLVertexShaderAddr(ss.str());
        }
    }

    return -1;
}
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(PixelShader, GenerateFromFile), MonoString* a_path)
{
    char* str = mono_string_to_utf8(a_path);

    const std::filesystem::path p = std::filesystem::path(str);

    mono_free(str);

    if (p.extension() == ".fpix" || p.extension() == ".ffrag")
    {
        std::ifstream file = std::ifstream(p);
        if (file.good() && file.is_open())
        {
            std::stringstream ss;

            ss << file.rdbuf();

            file.close();

            return Engine->GenerateFPixelShaderAddr(ss.str());
        }
    }
    else if (p.extension() == ".pix" || p.extension() == ".frag")
    {
        std::ifstream file = std::ifstream(p);
        if (file.good() && file.is_open())
        {
            std::stringstream ss;

            ss << file.rdbuf();

            file.close();

            return Engine->GenerateGLSLPixelShaderAddr(ss.str());
        }
    }

    return -1;
}

// Gonna leave theses functions seperate as there is a bit to it
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(Material, GenerateProgram), uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_vertexInputAttribs, MonoArray* a_shaderInputs, uint32_t a_cullingMode, uint32_t a_primitiveMode, uint32_t a_colorBlendingEnabled)
{
    FlareBase::RenderProgram program;
    program.VertexShader = a_vertexShader;
    program.PixelShader = a_pixelShader;
    program.VertexStride = a_vertexStride;
    program.CullingMode = (FlareBase::e_CullMode)a_cullingMode;
    program.PrimitiveMode = (FlareBase::e_PrimitiveMode)a_primitiveMode;
    program.EnableColorBlending = (uint8_t)a_colorBlendingEnabled;
    program.Flags = 0;

    // Need to recreate the array
    // Because it is a managed array may not be contiguous and is controlled by the GC 
    // Need a reliable lifetime and memory layout
    if (a_vertexInputAttribs != nullptr)
    {
        program.VertexInputCount = (uint16_t)mono_array_length(a_vertexInputAttribs);
        program.VertexAttribs = new FlareBase::VertexInputAttrib[program.VertexInputCount];

        for (uint16_t i = 0; i < program.VertexInputCount; ++i)
        {
            program.VertexAttribs[i] = mono_array_get(a_vertexInputAttribs, FlareBase::VertexInputAttrib, i);
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
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[program.ShaderBufferInputCount];

        for (uint16_t i = 0; i < program.ShaderBufferInputCount; ++i)
        {
            program.ShaderBufferInputs[i] = mono_array_get(a_shaderInputs, FlareBase::ShaderBufferInput, i);
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
    const FlareBase::RenderProgram program = Engine->GetRenderProgram(a_addr);

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

FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(Texture, GenerateFromFile), MonoString* a_path)
{
    char* str = mono_string_to_utf8(a_path);
    const std::filesystem::path p = std::filesystem::path(str);
    mono_free(str);

    uint32_t addr = -1;

    if (p.extension() == ".png")
    {
        int width;
        int height;
        int channels;

        stbi_uc* pixels = stbi_load(p.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
        if (pixels != nullptr)
        {
            addr = Engine->GenerateTexture((uint32_t)width, (uint32_t)height, pixels);

            stbi_image_free(pixels);
        }
    }

    return addr;
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
FLARE_MONO_EXPORT(uint32_t, RUNTIME_FUNCTION_NAME(Model, GenerateFromFile), MonoString* a_path)
{
    char* str = mono_string_to_utf8(a_path);

    uint32_t addr = -1;

    std::vector<FlareBase::Vertex> vertices;
    std::vector<uint32_t> indices;
    const std::filesystem::path p = std::filesystem::path(str);

    if (p.extension() == ".obj")
    {
        if (FlareBase::OBJLoader_LoadFile(p, &vertices, &indices))
        {
            addr = Engine->GenerateModel((const char*)vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(FlareBase::Vertex));
        }
    }
    else if (p.extension() == ".dae")
    {
        if (FlareBase::ColladaLoader_LoadFile(p, &vertices, &indices))
        {
            addr = Engine->GenerateModel((const char*)vertices.data(), (uint32_t)vertices.size(), indices.data(), (uint32_t)indices.size(), sizeof(FlareBase::Vertex));
        }
    }
    else
    {
        FLARE_ASSERT_MSG_R(0, "GenerateFromFile invalid file extension");
    }

    mono_free(str);

    return addr;
}
FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(Model, DestroyModel), uint32_t a_addr)
{
    Engine->DestroyModel(a_addr);
}

FLARE_MONO_EXPORT(void, RUNTIME_FUNCTION_NAME(RenderCommand, DrawModel), MonoArray* a_transform, uint32_t a_addr)
{
    glm::mat4 transform;

    float* f = (float*)&transform;
    for (int i = 0; i < 16; ++i)
    {
        f[i] = mono_array_get(a_transform, float, i);
    }

    Engine->DrawModel(transform, a_addr);
}

VulkanGraphicsEngineBindings::VulkanGraphicsEngineBindings(RuntimeManager* a_runtime, VulkanGraphicsEngine* a_graphicsEngine)
{
    m_graphicsEngine = a_graphicsEngine;

    Engine = this;

    TRACE("Binding Vulkan functions to C#");
    VULKANGRAPHICS_BINDING_FUNCTION_TABLE(VULKANGRAPHICS_RUNTIME_ATTACH)

    BIND_FUNCTION(a_runtime, FlareEngine.Rendering, VertexShader, GenerateFromFile);
    BIND_FUNCTION(a_runtime, FlareEngine.Rendering, PixelShader, GenerateFromFile);

    BIND_FUNCTION(a_runtime, FlareEngine.Rendering, Material, GenerateProgram);
    BIND_FUNCTION(a_runtime, FlareEngine.Rendering, Material, DestroyProgram);
    
    BIND_FUNCTION(a_runtime, FlareEngine.Rendering, Texture, GenerateFromFile);

    BIND_FUNCTION(a_runtime, FlareEngine.Rendering, Model, GenerateModel);
    BIND_FUNCTION(a_runtime, FlareEngine.Rendering, Model, GenerateFromFile);
    BIND_FUNCTION(a_runtime, FlareEngine.Rendering, Model, DestroyModel);

    BIND_FUNCTION(a_runtime, FlareEngine.Rendering, RenderCommand, DrawModel);
}
VulkanGraphicsEngineBindings::~VulkanGraphicsEngineBindings()
{

}

uint32_t VulkanGraphicsEngineBindings::GenerateFVertexShaderAddr(const std::string_view& a_str) const
{
    FLARE_ASSERT_MSG(!a_str.empty(), "GenerateFVertexShaderAddr empty string")

    VulkanVertexShader* shader = VulkanVertexShader::CreateFromFShader(m_graphicsEngine->m_vulkanEngine, a_str);

    uint32_t size = 0;
    {
        TLockArray<VulkanVertexShader*> a = m_graphicsEngine->m_vertexShaders.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = shader;

                return i;
            }
        }
    }
    
    m_graphicsEngine->m_vertexShaders.Push(shader);

    return size;
}
uint32_t VulkanGraphicsEngineBindings::GenerateGLSLVertexShaderAddr(const std::string_view& a_str) const
{
    FLARE_ASSERT_MSG(!a_str.empty(), "GenerateGLSLVertexShaderAddr empty string")

    VulkanVertexShader* shader = VulkanVertexShader::CreateFromGLSL(m_graphicsEngine->m_vulkanEngine, a_str);

    uint32_t size = 0;
    {
        TLockArray<VulkanVertexShader*> a = m_graphicsEngine->m_vertexShaders.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = shader;

                return i;
            }
        }
    }

    m_graphicsEngine->m_vertexShaders.Push(shader);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyVertexShader(uint32_t a_addr) const
{
    TLockArray<VulkanVertexShader*> a = m_graphicsEngine->m_vertexShaders.ToLockArray();

    FLARE_ASSERT_MSG(a_addr < a.Size(), "DestroyVertexShader out of bounds")
    FLARE_ASSERT_MSG(a[a_addr] != nullptr, "DestroyVertexShader already destroyed")

    delete a[a_addr];
    a[a_addr] = nullptr;
}

uint32_t VulkanGraphicsEngineBindings::GenerateFPixelShaderAddr(const std::string_view& a_str) const
{
    FLARE_ASSERT_MSG(!a_str.empty(), "GenerateFPixelShaderAddr empty string")

    VulkanPixelShader* shader = VulkanPixelShader::CreateFromFShader(m_graphicsEngine->m_vulkanEngine, a_str);

    uint32_t size = 0;
    {
        TLockArray<VulkanPixelShader*> a = m_graphicsEngine->m_pixelShaders.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = shader;

                return i;
            }
        }
    }

    m_graphicsEngine->m_pixelShaders.Push(shader);

    return size;
}
uint32_t VulkanGraphicsEngineBindings::GenerateGLSLPixelShaderAddr(const std::string_view& a_str) const
{
    FLARE_ASSERT_MSG(!a_str.empty(), "GenerateGLSLPixelShaderAddr empty string")

    VulkanPixelShader* shader = VulkanPixelShader::CreateFromGLSL(m_graphicsEngine->m_vulkanEngine, a_str);

    uint32_t size = 0;
    {
        TLockArray<VulkanPixelShader*> a = m_graphicsEngine->m_pixelShaders.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = shader;

                return i;
            }
        }
    }

    m_graphicsEngine->m_pixelShaders.Push(shader);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyPixelShader(uint32_t a_addr) const
{
    TLockArray<VulkanPixelShader*> a = m_graphicsEngine->m_pixelShaders.ToLockArray();

    FLARE_ASSERT_MSG(a_addr < a.Size(), "DestroyPixelShader out of bounds")
    FLARE_ASSERT_MSG(a[a_addr] != nullptr, "DestroyPixelShader already destroyed")

    delete a[a_addr];
    a[a_addr] = nullptr;
}

uint32_t VulkanGraphicsEngineBindings::GenerateInternalShaderProgram(FlareBase::e_InternalRenderProgram a_program) const
{
    FlareBase::RenderProgram program;
    program.VertexStride = 0;
    program.VertexInputCount = 0;
    program.VertexAttribs = nullptr;
    program.Flags |= 0b1 << FlareBase::RenderProgram::DestroyFlag;

    switch (a_program)
    {
    case FlareBase::InternalRenderProgram_DirectionalLight:
    {
        TRACE("Creating Directional Light Shader");
        program.VertexShader = GenerateGLSLVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(DIRECTIONALLIGHTPIXEL);
        program.CullingMode = FlareBase::CullMode_None;
        program.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
        program.EnableColorBlending = 1;

        constexpr uint32_t TextureCount = 5;
        constexpr uint32_t BufferCount = TextureCount + 2;

        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = FlareBase::ShaderBufferInput(i, FlareBase::ShaderBufferType_Texture, FlareBase::ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = FlareBase::ShaderBufferInput(TextureCount + 0, FlareBase::ShaderBufferType_DirectionalLightBuffer, FlareBase::ShaderSlot_Pixel, 1);
        program.ShaderBufferInputs[TextureCount + 1] = FlareBase::ShaderBufferInput(TextureCount + 1, FlareBase::ShaderBufferType_CameraBuffer, FlareBase::ShaderSlot_Pixel, 2);

        break;
    }
    case FlareBase::InternalRenderProgram_PointLight:
    {
        TRACE("Creating Point Light Shader");
        program.VertexShader = GenerateGLSLVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(POINTLIGHTPIXEL);
        program.CullingMode = FlareBase::CullMode_None;
        program.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
        program.EnableColorBlending = 1;

        constexpr uint32_t TextureCount = 5;
        constexpr uint32_t BufferCount = TextureCount + 2;
        
        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = FlareBase::ShaderBufferInput(i, FlareBase::ShaderBufferType_Texture, FlareBase::ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = FlareBase::ShaderBufferInput(TextureCount + 0, FlareBase::ShaderBufferType_PointLightBuffer, FlareBase::ShaderSlot_Pixel, 1);
        program.ShaderBufferInputs[TextureCount + 1] = FlareBase::ShaderBufferInput(TextureCount + 1, FlareBase::ShaderBufferType_CameraBuffer, FlareBase::ShaderSlot_Pixel, 2);

        break;
    }
    case FlareBase::InternalRenderProgram_SpotLight:
    {
        TRACE("Creating Spot Light Shader");
        program.VertexShader = GenerateGLSLVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(SPOTLIGHTPIXEL);
        program.CullingMode = FlareBase::CullMode_None;
        program.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
        program.EnableColorBlending = 1;

        constexpr uint32_t TextureCount = 5;
        constexpr uint32_t BufferCount = TextureCount + 2;

        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = FlareBase::ShaderBufferInput(i, FlareBase::ShaderBufferType_Texture, FlareBase::ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = FlareBase::ShaderBufferInput(TextureCount + 0, FlareBase::ShaderBufferType_SpotLightBuffer, FlareBase::ShaderSlot_Pixel, 1);
        program.ShaderBufferInputs[TextureCount + 1] = FlareBase::ShaderBufferInput(TextureCount + 1, FlareBase::ShaderBufferType_CameraBuffer, FlareBase::ShaderSlot_Pixel, 2);

        break;
    }
    case FlareBase::InternalRenderProgram_Post:
    {
        TRACE("Creating Post Shader");
        program.VertexShader = GenerateGLSLVertexShaderAddr(QUADVERTEX);
        program.PixelShader = GenerateFPixelShaderAddr(POSTPIXEL);
        program.CullingMode = FlareBase::CullMode_None;
        program.PrimitiveMode = FlareBase::PrimitiveMode_TriangleStrip;
        program.EnableColorBlending = 1;

        constexpr uint32_t TextureCount = 4;
        constexpr uint32_t BufferCount = TextureCount + 1;

        program.ShaderBufferInputCount = BufferCount;
        program.ShaderBufferInputs = new FlareBase::ShaderBufferInput[BufferCount];
        for (uint32_t i = 0; i < TextureCount; ++i)
        {
            program.ShaderBufferInputs[i] = FlareBase::ShaderBufferInput(i, FlareBase::ShaderBufferType_Texture, FlareBase::ShaderSlot_Pixel);
        }

        program.ShaderBufferInputs[TextureCount + 0] = FlareBase::ShaderBufferInput(TextureCount + 0, FlareBase::ShaderBufferType_CameraBuffer, FlareBase::ShaderSlot_Pixel, 1);

        break;
    }
    default:
    {
        FLARE_ASSERT_MSG(0, "Invalid Internal Render Program");
    }
    }

    return GenerateShaderProgram(program);
}
uint32_t VulkanGraphicsEngineBindings::GenerateShaderProgram(const FlareBase::RenderProgram& a_program) const
{
    FLARE_ASSERT_MSG(a_program.PixelShader < m_graphicsEngine->m_pixelShaders.Size(), "GenerateShaderProgram PixelShader out of bounds")
    FLARE_ASSERT_MSG(a_program.VertexShader < m_graphicsEngine->m_vertexShaders.Size(), "GenerateShaderProgram VertexShader out of bounds")

    uint32_t size = 0;
    TRACE("Creating Shader Program");
    {
        TLockArray<FlareBase::RenderProgram> a = m_graphicsEngine->m_shaderPrograms.ToLockArray();
        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].Flags & 0b1 << FlareBase::RenderProgram::FreeFlag)
            {
                FLARE_ASSERT_MSG(a[i].Data == nullptr, "GenerateShaderProgram data not deleted");

                a[i] = a_program;
                a[i].Data = new VulkanShaderData(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine, i);

                return i;
            }
        }
    }
    
    TRACE("Allocating Shader Program");
    m_graphicsEngine->m_shaderPrograms.Push(a_program);
    m_graphicsEngine->m_shaderPrograms[size].Data = new VulkanShaderData(m_graphicsEngine->m_vulkanEngine, m_graphicsEngine, size);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyShaderProgram(uint32_t a_addr) const
{
    TLockArray<FlareBase::RenderProgram> a = m_graphicsEngine->m_shaderPrograms.ToLockArray();

    FLARE_ASSERT_MSG(a_addr < a.Size(), "DestroyShaderProgram out of bounds");

    FlareBase::RenderProgram& program = a[a_addr];
    if (program.Flags & 0b1 << FlareBase::RenderProgram::DestroyFlag)
    {
        DestroyVertexShader(program.VertexShader);
        DestroyPixelShader(program.PixelShader);
    }
    program.Flags = 0b1 << FlareBase::RenderProgram::FreeFlag;

    if (program.Data != nullptr)
    {
        delete (VulkanShaderData*)program.Data;
        program.Data = nullptr;
    }
}
void VulkanGraphicsEngineBindings::RenderProgramSetTexture(uint32_t a_addr, uint32_t a_shaderSlot, uint32_t a_samplerAddr)
{
    TLockArray<FlareBase::RenderProgram> a = m_graphicsEngine->m_shaderPrograms.ToLockArray();

    FLARE_ASSERT_MSG(a_addr < a.Size(), "RenderProgramSetTexture material out of bounds");

    const FlareBase::RenderProgram& program = a[a_addr];

    FLARE_ASSERT_MSG(program.Data != nullptr, "RenderProgramSetTexture invalid program");

    VulkanShaderData* data = (VulkanShaderData*)program.Data;
    FLARE_ASSERT_MSG(a_samplerAddr < m_graphicsEngine->m_textureSampler.Size(), "RenderProgramSetTexture sampler out of bounds");
    data->SetTexture(a_shaderSlot, m_graphicsEngine->m_textureSampler[a_samplerAddr]);
}
FlareBase::RenderProgram VulkanGraphicsEngineBindings::GetRenderProgram(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "GetRenderProgram out of bounds")

    return m_graphicsEngine->m_shaderPrograms[a_addr];
}
void VulkanGraphicsEngineBindings::SetRenderProgram(uint32_t a_addr, const FlareBase::RenderProgram& a_program) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_shaderPrograms.Size(), "SetRenderProgram out of bounds")

    m_graphicsEngine->m_shaderPrograms[a_addr] = a_program;
}

uint32_t VulkanGraphicsEngineBindings::GenerateCameraBuffer(uint32_t a_transformAddr) const
{
    FLARE_ASSERT_MSG(a_transformAddr != -1, "GenerateCameraBuffer invalid transform address")

    const CameraBuffer buff = CameraBuffer(a_transformAddr);

    uint32_t size = 0;
    {
        TRACE("Getting Camera Buffer");
        TLockArray<CameraBuffer> a = m_graphicsEngine->m_cameraBuffers.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TransformAddr != -1)
            {
                a[i] = buff;

                return i;
            }
        }
    }
    

    TRACE("Allocating Camera Buffer");
    m_graphicsEngine->m_cameraBuffers.Push(buff);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyCameraBuffer(uint32_t a_addr) const
{
    TLockArray<CameraBuffer> a = m_graphicsEngine->m_cameraBuffers.ToLockArray();

    FLARE_ASSERT_MSG(a_addr < a.Size(), "DestroyCameraBuffer out of bounds")
    a[a_addr].TransformAddr = -1;
}
CameraBuffer VulkanGraphicsEngineBindings::GetCameraBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "GetCameraBuffer out of bounds")

    return m_graphicsEngine->m_cameraBuffers[a_addr];
}
void VulkanGraphicsEngineBindings::SetCameraBuffer(uint32_t a_addr, const CameraBuffer& a_buffer) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "SetCameraBuffer out of bounds")

    m_graphicsEngine->m_cameraBuffers.LockSet(a_addr, a_buffer);
}
glm::vec3 VulkanGraphicsEngineBindings::CameraScreenToWorld(uint32_t a_addr, const glm::vec3& a_screenPos, const glm::vec2& a_screenSize) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_cameraBuffers.Size(), "CameraScreenToWorld out of bounds");

    const CameraBuffer camBuf = m_graphicsEngine->m_cameraBuffers[a_addr];

    FLARE_ASSERT_MSG(camBuf.TransformAddr != -1, "CameraScreenToWorld invalid transform");

    const glm::mat4 proj = camBuf.ToProjection(a_screenSize);
    const glm::mat4 invProj = glm::inverse(proj);

    ObjectManager* objManager = m_graphicsEngine->m_vulkanEngine->GetRenderEngine()->GetObjectManager();
    const glm::mat4 invView = objManager->GetGlobalMatrix(camBuf.TransformAddr);

    const glm::vec4 cPos = invProj * glm::vec4(a_screenPos.xy() * 2.0f - 1.0f, a_screenPos.z, 1.0f);
    const glm::vec4 wPos = invView * cPos;

    return wPos.xyz() / wPos.w;
}

uint32_t VulkanGraphicsEngineBindings::GenerateModel(const char* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride) const
{
    FLARE_ASSERT_MSG(a_vertices != nullptr, "GenerateModel vertices null")
    FLARE_ASSERT_MSG(a_vertexCount > 0, "GenerateModel no vertices")
    FLARE_ASSERT_MSG(a_indices != nullptr, "GenerateModel indices null")
    FLARE_ASSERT_MSG(a_indexCount > 0, "GenerateModel no indices")
    FLARE_ASSERT_MSG(a_vertexStride > 0, "GenerateModel vertex stride 0")

    VulkanModel* model = new VulkanModel(m_graphicsEngine->m_vulkanEngine, a_vertexCount, a_vertices, a_vertexStride, a_indexCount, a_indices);

    uint32_t size = 0;
    {
        TLockArray<VulkanModel*> a = m_graphicsEngine->m_models.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = model;

                return i;
            }
        }
    }

    TRACE("Allocating Model Buffer");
    m_graphicsEngine->m_models.Push(model);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyModel(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_models.Size(), "DestroyModel out of bounds")

    VulkanModel* model = m_graphicsEngine->m_models[a_addr];

    FLARE_ASSERT_MSG(model != nullptr, "DestroyModel already destroyed")

    m_graphicsEngine->m_models[a_addr] = nullptr;
    delete model;
}

uint32_t VulkanGraphicsEngineBindings::GenerateMeshRenderBuffer(uint32_t a_materialAddr, uint32_t a_modelAddr, uint32_t a_transformAddr) const
{
    TRACE("Creating Render Buffer");
    const MeshRenderBuffer buffer = MeshRenderBuffer(a_materialAddr, a_modelAddr, a_transformAddr);

    uint32_t size = 0;
    {
        TLockArray<MeshRenderBuffer> a = m_graphicsEngine->m_renderBuffers.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].MaterialAddr == -1)
            {
                a[i] = buffer;

                return i;
            }
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

    {
        TLockArray<MaterialRenderStack> a = m_graphicsEngine->m_renderStacks.ToLockArray();

        const uint32_t size = a.Size();

        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].Add(buffer))
            {
                return;
            }
        }
    }
    

    TRACE("Allocating RenderStack");
    m_graphicsEngine->m_renderStacks.Push(buffer);
}
void VulkanGraphicsEngineBindings::DestroyRenderStack(uint32_t a_meshAddr) const
{
    FLARE_ASSERT_MSG(a_meshAddr < m_graphicsEngine->m_renderBuffers.Size(), "DestroyRenderStack out of bounds");

    TRACE("Removing RenderStack");
    const MeshRenderBuffer& buffer = m_graphicsEngine->m_renderBuffers[a_meshAddr];

    {
        TLockArray<MaterialRenderStack> a = m_graphicsEngine->m_renderStacks.ToLockArray();

        const uint32_t size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].Remove(buffer))
            {
                if (a[i].Empty())
                {
                    TRACE("Destroying RenderStack");
                    m_graphicsEngine->m_renderStacks.UErase(i);
                }

                return;
            }
        }
    }
}

uint32_t VulkanGraphicsEngineBindings::GenerateTexture(uint32_t a_width, uint32_t a_height, const void* a_data)
{
    VulkanTexture* texture = new VulkanTexture(m_graphicsEngine->m_vulkanEngine, a_width, a_height, a_data);

    uint32_t size = 0;
    {
        TLockArray<VulkanTexture*> a = m_graphicsEngine->m_textures.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = texture;

                return i;
            }
        }
    }

    m_graphicsEngine->m_textures.Push(texture);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyTexture(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_textures.Size(), "DestroyTexture Texture out of bounds");
    
    VulkanTexture* texture = m_graphicsEngine->m_textures[a_addr];

    FLARE_ASSERT_MSG(texture != nullptr, "DestroyTexture already destroyed");

    m_graphicsEngine->m_textures[a_addr] = nullptr;
    delete texture;
}

uint32_t VulkanGraphicsEngineBindings::GenerateTextureSampler(uint32_t a_texture, FlareBase::e_TextureFilter a_filter, FlareBase::e_TextureAddress a_addressMode) const
{
    FLARE_ASSERT_MSG(a_texture < m_graphicsEngine->m_textures.Size(), "GenerateTextureSampler Texture out of bounds");
    FLARE_ASSERT_MSG(m_graphicsEngine->m_textures[a_texture] != nullptr, "GenerateTextureSampler, Texture destroyed");

    FlareBase::TextureSampler sampler;
    sampler.Addr = a_texture;
    sampler.TextureMode = FlareBase::TextureMode_Texture;
    sampler.FilterMode = a_filter;
    sampler.AddressMode = a_addressMode;
    sampler.Data = new VulkanTextureSampler(m_graphicsEngine->m_vulkanEngine, sampler);

    uint32_t size = 0;
    {
        TLockArray<FlareBase::TextureSampler> a = m_graphicsEngine->m_textureSampler.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TextureMode == FlareBase::TextureMode_Null)
            {
                FLARE_ASSERT_MSG(a[i].Data == nullptr, "GenerateTextureSampler null sampler with data");

                a[i] = sampler;

                return i;
            }
        }
    }

    m_graphicsEngine->m_textureSampler.Push(sampler);

    return size;
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureSampler(uint32_t a_renderTexture, uint32_t a_textureIndex, FlareBase::e_TextureFilter a_filter, FlareBase::e_TextureAddress a_addressMode) const
{ 
    FLARE_ASSERT_MSG(a_renderTexture < m_graphicsEngine->m_renderTextures.Size(), "GenerateRenderTextureSampler RenderTexture out of bounds");
    FLARE_ASSERT_MSG(m_graphicsEngine->m_renderTextures[a_renderTexture] != nullptr, "GenerateRenderTextureSampler RenderTexture destroyed");
    FLARE_ASSERT_MSG(a_textureIndex < m_graphicsEngine->m_renderTextures[a_renderTexture]->GetTextureCount(), "GenerateRenderTextureSampler texture index out of bounds");

    FlareBase::TextureSampler sampler;
    sampler.Addr = a_renderTexture;
    sampler.TextureMode = FlareBase::TextureMode_RenderTexture;
    sampler.TSlot = a_textureIndex;
    sampler.FilterMode = a_filter;
    sampler.AddressMode = a_addressMode;
    sampler.Data = new VulkanTextureSampler(m_graphicsEngine->m_vulkanEngine, sampler);

    uint32_t size = 0;
    {
        TLockArray<FlareBase::TextureSampler> a = m_graphicsEngine->m_textureSampler.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TextureMode == FlareBase::TextureMode_Null)
            {
                FLARE_ASSERT_MSG(a[i].Data == nullptr, "GenerateRenderTextureSampler null sampler with data");

                a[i] = sampler;

                return i;
            }
        }
    }    

    m_graphicsEngine->m_textureSampler.Push(sampler);

    return size;
}
uint32_t VulkanGraphicsEngineBindings::GenerateRenderTextureDepthSampler(uint32_t a_renderTexture, FlareBase::e_TextureFilter a_filter, FlareBase::e_TextureAddress a_addressMode) const
{
    FLARE_ASSERT_MSG(a_renderTexture < m_graphicsEngine->m_renderTextures.Size(), "GenerateRenderTextureDepthSampler out of bounds");
    FLARE_ASSERT_MSG(m_graphicsEngine->m_renderTextures[a_renderTexture] != nullptr, "GenerateRenderTextureDepthSampler RenderTexture destroyed");

    FlareBase::TextureSampler sampler;
    sampler.Addr = a_renderTexture;
    sampler.TextureMode = FlareBase::TextureMode_RenderTextureDepth;
    sampler.FilterMode = a_filter;
    sampler.AddressMode = a_addressMode;
    sampler.Data = new VulkanTextureSampler(m_graphicsEngine->m_vulkanEngine, sampler);

    uint32_t size = 0;
    {
        TLockArray<FlareBase::TextureSampler> a = m_graphicsEngine->m_textureSampler.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TextureMode == FlareBase::TextureMode_Null)
            {
                FLARE_ASSERT_MSG(a[i].Data == nullptr, "GenerateRenderTextureDepthSampler null sampler with data")

                a[i] = sampler;

                return i;
            }
        }
    }

    m_graphicsEngine->m_textureSampler.Push(sampler);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyTextureSampler(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_textureSampler.Size(), "DestroyTextureSampler out of bounds");
    
    FlareBase::TextureSampler nullSampler;
    nullSampler.TextureMode = FlareBase::TextureMode_Null;
    nullSampler.Data = nullptr;

    const FlareBase::TextureSampler sampler = m_graphicsEngine->m_textureSampler[a_addr];

    m_graphicsEngine->m_textureSampler.LockSet(a_addr, nullSampler);

    if (sampler.Data != nullptr)
    {
        delete (VulkanTextureSampler*)sampler.Data;
    }
}

uint32_t VulkanGraphicsEngineBindings::GenerateRenderTexture(uint32_t a_count, uint32_t a_width, uint32_t a_height, bool a_depthTexture, bool a_hdr) const
{
    FLARE_ASSERT_MSG(a_count > 0, "GenerateRenderTexture no textures");
    FLARE_ASSERT_MSG(a_width > 0, "GenerateRenderTexture width 0");
    FLARE_ASSERT_MSG(a_height > 0, "GenerateRenderTexture height 0");

    VulkanRenderEngineBackend* engine = m_graphicsEngine->m_vulkanEngine;

    VulkanRenderTexture* texture = new VulkanRenderTexture(engine, a_count, a_width, a_height, a_depthTexture, a_hdr);

    uint32_t size = 0;
    {
        TLockArray<VulkanRenderTexture*> a = m_graphicsEngine->m_renderTextures.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i] == nullptr)
            {
                a[i] = texture;

                return i;
            }
        }
    }

    TRACE("Allocating RenderTexture Buffer");
    m_graphicsEngine->m_renderTextures.Push(texture);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyRenderTexture(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_renderTextures.Size(), "DestroyRenderTexture out of bounds");

    VulkanRenderTexture* tex = m_graphicsEngine->m_renderTextures[a_addr];

    m_graphicsEngine->m_renderTextures[a_addr] = nullptr;

    delete tex;
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

    uint32_t size = 0;
    {
        TLockArray<DirectionalLightBuffer> a = m_graphicsEngine->m_directionalLights.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TransformAddr != -1)
            {
                a[i] = buffer;

                return i;
            }
        }
    }

    TRACE("Allocating DirectionalLight Buffer");
    m_graphicsEngine->m_directionalLights.Push(buffer);

    return size;
}
void VulkanGraphicsEngineBindings::SetDirectionalLightBuffer(uint32_t a_addr, const DirectionalLightBuffer& a_buffer) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "SetDirectionalLightBuffer out of bounds");

    m_graphicsEngine->m_directionalLights.LockSet(a_addr, a_buffer);
}
DirectionalLightBuffer VulkanGraphicsEngineBindings::GetDirectionalLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "GetDirectionalLightBuffer out of bounds");

    return m_graphicsEngine->m_directionalLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroyDirectionalLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_directionalLights.Size(), "DestroyDirectionalLightBuffer out of bounds");

    m_graphicsEngine->m_directionalLights.LockSet(a_addr, DirectionalLightBuffer(-1));
}

uint32_t VulkanGraphicsEngineBindings::GeneratePointLightBuffer(uint32_t a_transformAddr) const
{
    const PointLightBuffer buffer = PointLightBuffer(a_transformAddr);

    FLARE_ASSERT_MSG(buffer.TransformAddr != -1, "GeneratePointLightBuffer no transform");

    uint32_t size = 0;
    {
        TLockArray<PointLightBuffer> a = m_graphicsEngine->m_pointLights.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TransformAddr != -1)
            {
                a[i] = buffer;

                return i;
            }
        }
    }

    TRACE("Allocating PointLight Buffer");
    m_graphicsEngine->m_pointLights.Push(buffer);

    return size;
}
void VulkanGraphicsEngineBindings::SetPointLightBuffer(uint32_t a_addr, const PointLightBuffer& a_buffer) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "SetPointLightBuffer out of bounds");

    m_graphicsEngine->m_pointLights.LockSet(a_addr, a_buffer);
}
PointLightBuffer VulkanGraphicsEngineBindings::GetPointLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "GetPointLightBuffer out of bounds");

    return m_graphicsEngine->m_pointLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroyPointLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_pointLights.Size(), "DestroyPointLightBuffer out of bounds");

    m_graphicsEngine->m_pointLights.LockSet(a_addr, PointLightBuffer(-1));
}

uint32_t VulkanGraphicsEngineBindings::GenerateSpotLightBuffer(uint32_t a_transformAddr) const
{
    const SpotLightBuffer buffer = SpotLightBuffer(a_transformAddr);

    FLARE_ASSERT_MSG(buffer.TransformAddr != -1, "GenerateSpotLightBuffer no tranform");

    uint32_t size = 0;
    {
        TLockArray<SpotLightBuffer> a = m_graphicsEngine->m_spotLights.ToLockArray();

        size = a.Size();
        for (uint32_t i = 0; i < size; ++i)
        {
            if (a[i].TransformAddr != -1)
            {
                a[i] = buffer;

                return i;
            }
        }
    }

    TRACE("Allocating SpotLight Buffer");
    m_graphicsEngine->m_spotLights.Push(buffer);

    return size;
}
void VulkanGraphicsEngineBindings::SetSpotLightBuffer(uint32_t a_addr, const SpotLightBuffer& a_buffer) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "SetSpotLightBuffer out of bounds");

    m_graphicsEngine->m_spotLights.LockSet(a_addr, a_buffer);
}
SpotLightBuffer VulkanGraphicsEngineBindings::GetSpotLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "GetSpotLightBuffer out of bounds");

    return m_graphicsEngine->m_spotLights[a_addr];
}
void VulkanGraphicsEngineBindings::DestroySpotLightBuffer(uint32_t a_addr) const
{
    FLARE_ASSERT_MSG(a_addr < m_graphicsEngine->m_spotLights.Size(), "DestroySpotLightBuffer out of bounds");

    m_graphicsEngine->m_spotLights.LockSet(a_addr, SpotLightBuffer(-1));
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
void VulkanGraphicsEngineBindings::DrawMaterial()
{
    FLARE_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "DrawMaterial RenderCommand does not exist");

    m_graphicsEngine->m_renderCommands->DrawMaterial();
}
void VulkanGraphicsEngineBindings::DrawModel(const glm::mat4& a_transform, uint32_t a_addr)
{
    FLARE_ASSERT_MSG(m_graphicsEngine->m_renderCommands.Exists(), "DrawModel RenderCommand does not exist");

    m_graphicsEngine->m_renderCommands->DrawModel(a_transform, a_addr);
}