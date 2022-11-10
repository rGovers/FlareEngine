#include "Rendering/Vulkan/VulkanGraphicsEngineBindings.h"

#include "RuntimeManager.h"
#include "Rendering/Vulkan/VulkanGraphicsEngine.h"
#include "Rendering/Vulkan/VulkanModel.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "Trace.h"

static VulkanGraphicsEngineBindings* Engine = nullptr;

static uint32_t VertexShader_GenerateShader(MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    const uint32_t ret = Engine->GenerateVertexShaderAddr(str);

    mono_free(str);

    return ret;
}
static void VertexShader_DestroyShader(uint32_t a_addr)
{
    Engine->DestroyVertexShader(a_addr);
}

static uint32_t PixelShader_GenerateShader(MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    const uint32_t ret = Engine->GeneratePixelShaderAddr(str);

    mono_free(str);

    return ret;
}
static void PixelShader_DestroyShader(uint32_t a_addr)
{
    Engine->DestroyPixelShader(a_addr);
}

static uint32_t Material_GenerateProgram(uint32_t a_vertexShader, uint32_t a_pixelShader, uint16_t a_vertexStride, MonoArray* a_vertexInputAttribs, MonoArray* a_shaderInputs)
{
    RenderProgram program;
    program.VertexShader = a_vertexShader;
    program.PixelShader = a_pixelShader;
    program.VertexStride = a_vertexStride;

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
static void Material_DestroyProgram(uint32_t a_addr)
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
static RenderProgram Material_GetProgramBuffer(uint32_t a_addr)
{
    return Engine->GetRenderProgram(a_addr);
}
static void Material_SetProgramBuffer(uint32_t a_addr, RenderProgram a_program)
{
    Engine->SetRenderProgram(a_addr, a_program);
}

static uint32_t Camera_GenerateBuffer(uint32_t a_transformAddr)
{
    return Engine->GenerateCameraBuffer(a_transformAddr);
}
static void Camera_DestroyBuffer(uint32_t a_addr)
{
    Engine->DestroyCameraBuffer(a_addr);
}
static CameraBuffer Camera_GetBuffer(uint32_t a_addr)
{
    return Engine->GetCameraBuffer(a_addr);
}
static void Camera_SetBuffer(uint32_t a_addr, CameraBuffer a_buffer)
{
    Engine->SetCameraBuffer(a_addr, a_buffer);
}

static uint32_t MeshRenderer_GenerateBuffer(uint32_t a_transformAddr, uint32_t a_materialAddr, uint32_t a_modelAddr)
{
    MeshRenderBuffer buffer;
    buffer.MaterialAddr = a_materialAddr;
    buffer.ModelAddr = a_modelAddr;
    buffer.TransformAddr = a_transformAddr;

    return Engine->GenerateMeshRenderBuffer(buffer);
}
static void MeshRenderer_DestroyBuffer(uint32_t a_addr)
{
    Engine->DestroyMeshRenderBuffer(a_addr);
}
static void MeshRenderer_GenerateRenderStack(uint32_t a_addr)
{
    Engine->GenerateRenderStack(a_addr);
}
static void MeshRenderer_DestroyRenderStack(uint32_t a_addr)
{
    Engine->DestroyRenderStack(a_addr);
}

static uint32_t Model_GenerateModel(MonoArray* a_vertices, MonoArray* a_indices, uint16_t a_vertexStride)
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
static void Model_DestroyModel(uint32_t a_addr)
{
    Engine->DestroyModel(a_addr);
}

VulkanGraphicsEngineBindings::VulkanGraphicsEngineBindings(RuntimeManager* a_runtime, VulkanGraphicsEngine* a_graphicsEngine)
{
    m_graphicsEngine = a_graphicsEngine;

    Engine = this;

    TRACE("Binding Vulkan functions to C#");
    a_runtime->BindFunction("FlareEngine.Rendering.VertexShader::GenerateShader", (void*)VertexShader_GenerateShader);
    a_runtime->BindFunction("FlareEngine.Rendering.VertexShader::DestroyShader", (void*)VertexShader_DestroyShader);
    a_runtime->BindFunction("FlareEngine.Rendering.PixelShader::GenerateShader", (void*)PixelShader_GenerateShader);
    a_runtime->BindFunction("FlareEngine.Rendering.PixelShader::DestroyShader", (void*)PixelShader_DestroyShader);    

    a_runtime->BindFunction("FlareEngine.Rendering.Material::GenerateProgram", (void*)Material_GenerateProgram);
    a_runtime->BindFunction("FlareEngine.Rendering.Material::DestroyProgram", (void*)Material_DestroyProgram);
    a_runtime->BindFunction("FlareEngine.Rendering.Material::GetProgramBuffer", (void*)Material_GetProgramBuffer);
    a_runtime->BindFunction("FlareEngine.Rendering.Material::SetProgramBuffer", (void*)Material_SetProgramBuffer);

    a_runtime->BindFunction("FlareEngine.Rendering.Camera::GenerateBuffer", (void*)Camera_GenerateBuffer);
    a_runtime->BindFunction("FlareEngine.Rendering.Camera::DestroyBuffer", (void*)Camera_DestroyBuffer);
    a_runtime->BindFunction("FlareEngine.Rendering.Camera::GetBuffer", (void*)Camera_GetBuffer);
    a_runtime->BindFunction("FlareEngine.Rendering.Camera::SetBuffer", (void*)Camera_SetBuffer);

    a_runtime->BindFunction("FlareEngine.Rendering.Model::GenerateModel", (void*)Model_GenerateModel);
    a_runtime->BindFunction("FlareEngine.Rendering.Model::DestroyModel", (void*)Model_DestroyModel);

    a_runtime->BindFunction("FlareEngine.Rendering.MeshRenderer::GenerateBuffer", (void*)MeshRenderer_GenerateBuffer);
    a_runtime->BindFunction("FlareEngine.Rendering.MeshRenderer::DestroyBuffer", (void*)MeshRenderer_DestroyBuffer);
    a_runtime->BindFunction("FlareEngine.Rendering.MeshRenderer::GenerateRenderStack", (void*)MeshRenderer_GenerateRenderStack);
    a_runtime->BindFunction("FlareEngine.Rendering.MeshRenderer::DestroyRenderStack", (void*)MeshRenderer_DestroyRenderStack);
}
VulkanGraphicsEngineBindings::~VulkanGraphicsEngineBindings()
{

}

uint32_t VulkanGraphicsEngineBindings::GenerateVertexShaderAddr(const std::string_view& a_str)
{
    VulkanVertexShader* shader = VulkanVertexShader::CreateFromGLSL(m_graphicsEngine->m_vulkanEngine, a_str);

    const uint32_t size = (uint32_t)m_graphicsEngine->m_vertexShaders.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_vertexShaders[i] == nullptr)
        {
            m_graphicsEngine->m_vertexShaders[i] = shader;
            
            return i;
        }
    }

    m_graphicsEngine->m_vertexShaders.emplace_back(shader);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyVertexShader(uint32_t a_addr)
{
    if (m_graphicsEngine->m_vertexShaders[a_addr] != nullptr)
    {
        delete m_graphicsEngine->m_vertexShaders[a_addr];
        m_graphicsEngine->m_vertexShaders[a_addr] = nullptr;
    }
    else
    {
        printf("VertexShader already destroyed \n");

        assert(0);
    }
}

uint32_t VulkanGraphicsEngineBindings::GeneratePixelShaderAddr(const std::string_view& a_str)
{
    VulkanPixelShader* shader = VulkanPixelShader::CreateFromGLSL(m_graphicsEngine->m_vulkanEngine, a_str);

    const uint32_t size = (uint32_t)m_graphicsEngine->m_pixelShaders.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_pixelShaders[i] == nullptr)
        {
            m_graphicsEngine->m_pixelShaders[i] = shader;

            return i;
        }
    }

    m_graphicsEngine->m_pixelShaders.emplace_back(shader);

    return size;
}

void VulkanGraphicsEngineBindings::DestroyPixelShader(uint32_t a_addr)
{
    if (m_graphicsEngine->m_pixelShaders[a_addr] != nullptr)
    {
        delete m_graphicsEngine->m_pixelShaders[a_addr];
        m_graphicsEngine->m_pixelShaders[a_addr] = nullptr;
    }
    else
    {
        printf("PixelShader already destroyed \n");

        assert(0);
    }
}

uint32_t VulkanGraphicsEngineBindings::GenerateShaderProgram(const RenderProgram& a_program)
{
    TRACE("Creating Shader Program");

    if (a_program.PixelShader > m_graphicsEngine->m_pixelShaders.size() || a_program.VertexShader > m_graphicsEngine->m_vertexShaders.size())
    {
        printf("Invalid ShaderProgram \n");

        assert(0);
    }

    if (m_graphicsEngine->m_freeShaderSlots.size() > 0)
    {
        const uint32_t addr = m_graphicsEngine->m_freeShaderSlots.front();
        m_graphicsEngine->m_freeShaderSlots.pop();

        return addr;
    }

    TRACE("Allocating Shader Program");

    m_graphicsEngine->m_shaderPrograms.emplace_back(a_program);

    return (uint32_t)m_graphicsEngine->m_shaderPrograms.size() - 1;
}
void VulkanGraphicsEngineBindings::DestroyShaderProgram(uint32_t a_addr)
{
    m_graphicsEngine->m_freeShaderSlots.emplace(a_addr);

    if (m_graphicsEngine->m_freeShaderSlots.size() == m_graphicsEngine->m_shaderPrograms.size())
    {
        m_graphicsEngine->m_shaderPrograms.clear();
        m_graphicsEngine->m_freeShaderSlots = std::queue<uint32_t>();
    }
}
RenderProgram VulkanGraphicsEngineBindings::GetRenderProgram(uint32_t a_addr) const
{
    return m_graphicsEngine->m_shaderPrograms[a_addr];
}
void VulkanGraphicsEngineBindings::SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program)
{
    m_graphicsEngine->m_shaderPrograms[a_addr] = a_program;
}

uint32_t VulkanGraphicsEngineBindings::GenerateCameraBuffer(uint32_t a_transformAddr)
{
    CameraBuffer buff;
    buff.TransformAddr = a_transformAddr;

    TRACE("Getting Camera Buffer");
    const uint32_t size = (uint32_t)m_graphicsEngine->m_cameraBuffers.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_cameraBuffers[i].TransformAddr == -1)
        {
            m_graphicsEngine->m_cameraBuffers[i] = buff;

            return i;
        }
    }

    TRACE("Allocating Camera Buffer");
    m_graphicsEngine->m_cameraBuffers.emplace_back(buff);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyCameraBuffer(uint32_t a_addr)
{
    m_graphicsEngine->m_cameraBuffers[a_addr].TransformAddr = -1;

    uint32_t val = (--m_graphicsEngine->m_cameraBuffers.end())->TransformAddr;
    while (val == -1)
    {
        TRACE("Destroying Camera Buffer");
        m_graphicsEngine->m_cameraBuffers.pop_back();

        if (m_graphicsEngine->m_cameraBuffers.empty())
        {
            break;
        }

        val = (--m_graphicsEngine->m_cameraBuffers.end())->TransformAddr;
    }
}
CameraBuffer VulkanGraphicsEngineBindings::GetCameraBuffer(uint32_t a_addr) const
{
    return m_graphicsEngine->m_cameraBuffers[a_addr];
}
void VulkanGraphicsEngineBindings::SetCameraBuffer(uint32_t a_addr, const CameraBuffer& a_buffer)
{
    m_graphicsEngine->m_cameraBuffers[a_addr] = a_buffer;
}

uint32_t VulkanGraphicsEngineBindings::GenerateModel(const char* a_vertices, uint32_t a_vertexCount, const uint32_t* a_indices, uint32_t a_indexCount, uint16_t a_vertexStride)
{
    VulkanModel* model = new VulkanModel(m_graphicsEngine->m_vulkanEngine, a_vertexCount, a_vertices, a_vertexStride, a_indexCount, a_indices);

    const uint32_t size = (uint32_t)m_graphicsEngine->m_models.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_models[i] == nullptr)
        {
            m_graphicsEngine->m_models[i] = model;

            return i;
        }
    }

    m_graphicsEngine->m_models.emplace_back(model);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyModel(uint32_t a_addr)
{
    if (m_graphicsEngine->m_models[a_addr] != nullptr)
    {
        delete m_graphicsEngine->m_models[a_addr];
        m_graphicsEngine->m_models[a_addr] = nullptr;
    }
    else
    {
        printf("Model already destroyed \n");

        assert(0);
    }
}

uint32_t VulkanGraphicsEngineBindings::GenerateMeshRenderBuffer(const MeshRenderBuffer& a_renderBuffer)
{
    TRACE("Creating Render Buffer");
    const uint32_t size = (uint32_t)m_graphicsEngine->m_renderBuffers.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_graphicsEngine->m_renderBuffers[i].MaterialAddr == -1)
        {
            m_graphicsEngine->m_renderBuffers[i] = a_renderBuffer;

            return i;
        }
    }

    TRACE("Allocating Render Buffer");
    m_graphicsEngine->m_renderBuffers.emplace_back(a_renderBuffer);

    return size;
}
void VulkanGraphicsEngineBindings::DestroyMeshRenderBuffer(uint32_t a_addr)
{
    TRACE("Destroying Render Buffer");
    m_graphicsEngine->m_renderBuffers[a_addr].MaterialAddr = -1;
}
void VulkanGraphicsEngineBindings::GenerateRenderStack(uint32_t a_meshAddr)
{
    TRACE("Pushing RenderStack");
    const MeshRenderBuffer& buffer = m_graphicsEngine->m_renderBuffers[a_meshAddr];

    for (auto iter = m_graphicsEngine->m_renderStacks.begin(); iter != m_graphicsEngine->m_renderStacks.end(); ++iter)
    {
        if (iter->Add(buffer))
        {
            return;
        }
    }

    TRACE("Allocating RenderStack");
    m_graphicsEngine->m_renderStacks.emplace_back(buffer);
}
void VulkanGraphicsEngineBindings::DestroyRenderStack(uint32_t a_meshAddr)
{
    TRACE("Removing RenderStack");
    const MeshRenderBuffer& buffer = m_graphicsEngine->m_renderBuffers[a_meshAddr];

    for (auto iter = m_graphicsEngine->m_renderStacks.begin(); iter != m_graphicsEngine->m_renderStacks.end(); ++iter)
    {
        if (iter->Remove(buffer))
        {
            if (iter->Empty())
            {
                TRACE("Destroying RenderStack");
                m_graphicsEngine->m_renderStacks.erase(iter);
            }

            return;
        }
    }    
}