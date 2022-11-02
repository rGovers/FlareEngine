#include "Rendering/Vulkan/VulkanGraphicsEngine.h"

#include "Rendering/Vulkan/VulkanPipeline.h"
#include "Rendering/Vulkan/VulkanPixelShader.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanVertexShader.h"
#include "RuntimeManager.h"
#include "Trace.h"

static VulkanGraphicsEngine* Engine = nullptr;

static uint32_t VertexShader_GenerateShader(MonoString* a_string)
{
    char* str = mono_string_to_utf8(a_string);

    const uint32_t ret = Engine->GenerateVertexShaderAddr(str);

    free(str);

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

    free(str);

    return ret;
}
static void PixelShader_DestroyShader(uint32_t a_addr)
{
    Engine->DestroyPixelShader(a_addr);
}

static uint32_t Material_GenerateProgram(uint32_t a_vertexShader, uint32_t a_pixelShader)
{
    RenderProgram program;
    program.VertexShader = a_vertexShader;
    program.PixelShader = a_pixelShader;

    return Engine->GenerateShaderProgram(program);
}
static void Material_DestroyProgram(uint32_t a_addr)
{
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

static uint32_t Camera_GenerateBuffer()
{
    return Engine->GenerateCameraBuffer();
}
static void Camera_DestroyBuffer(uint32_t a_addr)
{
    Engine->DestroyCameraBuffer(a_addr);
}
static CameraBuffer Camera_GetBuffer(uint32_t a_addr)
{
    return Engine->GetCameraBuffer(a_addr);
}
static CameraBuffer Camera_SetBuffer(uint32_t a_addr, CameraBuffer a_buffer)
{
    Engine->SetCameraBuffer(a_addr, a_buffer);
}

VulkanGraphicsEngine::VulkanGraphicsEngine(RuntimeManager* a_runtime, VulkanRenderEngineBackend* a_vulkanEngine)
{
    m_vulkanEngine = a_vulkanEngine;

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
}
VulkanGraphicsEngine::~VulkanGraphicsEngine()
{
    TRACE("Deleting Pipelines")
    for (const auto& iter : m_pipelines)
    {
        delete iter.second;
    }

    TRACE("Checking if shaders where deleted");
    for (const VulkanVertexShader* shader : m_vertexShaders)
    {
        if (shader != nullptr)
        {
            printf("Vertex Shader was not destroyed \n");

            delete shader;
        }
    }

    for (const VulkanPixelShader* shader : m_pixelShaders)
    {
        if (shader != nullptr)
        {
            printf("Pixel Shader was not destroyed \n");

            delete shader;
        }
    }

    TRACE("Checking camera buffer health");
    if (m_freeCamSlots.size() != m_cameraBuffers.size())
    {
        printf("Camera buffers out of sync \n");
        printf("Likely camera leaked \n");
    }

    TRACE("Checking shader program buffer health");
    if (m_freeShaderSlots.size() != m_shaderPrograms.size())
    {
        printf("Shader buffers out of sync \n");
        printf("Likely material leaked \n");
    }
}

void VulkanGraphicsEngine::Update(VulkanSwapchain* a_swapchain)
{
    // TODO: Rewrite down the line
    const uint32_t camBufferSize = (uint32_t)m_cameraBuffers.size();
    const uint32_t renderBufferSize = (uint32_t)m_shaderPrograms.size();
    for (uint32_t i = 0; i < camBufferSize; ++i)
    {
        const CameraBuffer& camBuffer = m_cameraBuffers[i];

        for (uint32_t j = 0; j < renderBufferSize; ++j)
        {
            const RenderProgram& program = m_shaderPrograms[j];

            if (camBuffer.RenderLayer & program.RenderLayer)
            {
                const uint64_t ind = i | j << 32;
                const auto iter = m_pipelines.find(ind);
                if (iter == m_pipelines.end())
                {
                    TRACE("Creating Vulkan Pipeline");
                    m_pipelines.emplace(ind, new VulkanPipeline(m_vulkanEngine, this, a_swapchain->GetRenderPass(), i, program));
                }
            }
        }
    }

    for (auto iter : m_pipelines)
    {
        int brk = 3;
    }
}

uint32_t VulkanGraphicsEngine::GenerateVertexShaderAddr(const std::string_view& a_str)
{
    VulkanVertexShader* shader = VulkanVertexShader::CreateFromGLSL(m_vulkanEngine, a_str);

    const uint32_t size = (uint32_t)m_vertexShaders.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_vertexShaders[i] == nullptr)
        {
            m_vertexShaders[i] = shader;
            
            return i;
        }
    }

    m_vertexShaders.emplace_back(shader);

    return size;
}
VulkanVertexShader* VulkanGraphicsEngine::GetVertexShader(uint32_t a_addr) const
{
    return m_vertexShaders[a_addr];
}
void VulkanGraphicsEngine::DestroyVertexShader(uint32_t a_addr)
{
    if (m_vertexShaders[a_addr] != nullptr)
    {
        delete m_vertexShaders[a_addr];
        m_vertexShaders[a_addr] = nullptr;
    }
    else
    {
        printf("VertexShader already destroyed \n");

        assert(0);
    }
}

uint32_t VulkanGraphicsEngine::GeneratePixelShaderAddr(const std::string_view& a_str)
{
    VulkanPixelShader* shader = VulkanPixelShader::CreateFromGLSL(m_vulkanEngine, a_str);

    const uint32_t size = (uint32_t)m_pixelShaders.size();
    for (uint32_t i = 0; i < size; ++i)
    {
        if (m_pixelShaders[i] == nullptr)
        {
            m_pixelShaders[i] = shader;

            return i;
        }
    }

    m_pixelShaders.emplace_back(shader);

    return size;
}
VulkanPixelShader* VulkanGraphicsEngine::GetPixelShader(uint32_t a_addr) const
{
    return m_pixelShaders[a_addr];
}
void VulkanGraphicsEngine::DestroyPixelShader(uint32_t a_addr)
{
    if (m_pixelShaders[a_addr] != nullptr)
    {
        delete m_pixelShaders[a_addr];
        m_pixelShaders[a_addr] = nullptr;
    }
    else
    {
        printf("PixelShader already destroyed \n");

        assert(0);
    }
}

uint32_t VulkanGraphicsEngine::GenerateShaderProgram(const RenderProgram& a_program)
{
    TRACE("Creating Shader Program");

    if (a_program.PixelShader > m_pixelShaders.size() || a_program.VertexShader > m_vertexShaders.size())
    {
        printf("Invalid ShaderProgram \n");

        assert(0);
    }

    if (m_freeShaderSlots.size() > 0)
    {
        const uint32_t addr = m_freeShaderSlots.front();
        m_freeShaderSlots.pop();

        return addr;
    }

    TRACE("Allocating Shader Program");

    m_shaderPrograms.emplace_back(a_program);

    return (uint32_t)m_shaderPrograms.size() - 1;
}
void VulkanGraphicsEngine::DestroyShaderProgram(uint32_t a_addr)
{
    m_freeShaderSlots.emplace(a_addr);

    if (m_freeShaderSlots.size() == m_shaderPrograms.size())
    {
        m_shaderPrograms.clear();
        m_freeShaderSlots = std::queue<uint32_t>();
    }
}
RenderProgram VulkanGraphicsEngine::GetRenderProgram(uint32_t a_addr) const
{
    return m_shaderPrograms[a_addr];
}
void VulkanGraphicsEngine::SetRenderProgram(uint32_t a_addr, const RenderProgram& a_program)
{
    m_shaderPrograms[a_addr] = a_program;
}

uint32_t VulkanGraphicsEngine::GenerateCameraBuffer()
{
    TRACE("Getting Camera Buffer");
    if (m_freeCamSlots.size() > 0)
    {
        const uint32_t addr = m_freeCamSlots.front();
        m_freeCamSlots.pop();

        return addr;
    }

    TRACE("Allocating Camera Buffer");

    m_cameraBuffers.emplace_back();

    return (uint32_t)m_cameraBuffers.size() - 1;
}
void VulkanGraphicsEngine::DestroyCameraBuffer(uint32_t a_addr)
{
    m_freeCamSlots.emplace(a_addr);

    if (m_freeCamSlots.size() == m_cameraBuffers.size())
    {
        m_cameraBuffers.clear();
        m_freeCamSlots = std::queue<uint32_t>();
    }
}
CameraBuffer VulkanGraphicsEngine::GetCameraBuffer(uint32_t a_addr) const
{
    return m_cameraBuffers[a_addr];
}
void VulkanGraphicsEngine::SetCameraBuffer(uint32_t a_addr, const CameraBuffer& a_buffer)
{
    m_cameraBuffers[a_addr] = a_buffer;
}