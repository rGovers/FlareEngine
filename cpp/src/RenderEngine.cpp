#include "Rendering/RenderEngine.h"

#include <assert.h>

#include "Config.h"
#include "Rendering/SpirvTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"

RenderEngine::RenderEngine(GLFWwindow* a_window, Config* a_config)
{
    m_config = a_config;

    m_window = a_window;

    assert(m_config->GetRenderingEngine() != RenderingEngine_Null);

    switch (m_config->GetRenderingEngine())
    {
    case RenderingEngine_Vulkan:
    {
        m_backend = new VulkanRenderEngineBackend(this);

        break;
    }
    default:
    {
        assert(0);

        break;
    }
    }

    spirv_init();
}
RenderEngine::~RenderEngine()
{
    spirv_destroy();

    delete m_backend;
}

void RenderEngine::Update()
{
    m_backend->Update();
}

uint32_t RenderEngine::GenerateVertexShaderAddr(const std::string_view& a_str)
{
    return m_backend->GenerateVertexShaderAddr(a_str);
}
void RenderEngine::DestroyVertexShader(uint32_t a_addr)
{
    m_backend->DestoryVertexShader(a_addr);
}

uint32_t RenderEngine::GeneratePixelShaderAddr(const std::string_view& a_str)
{
    return m_backend->GeneratePixelShaderAddr(a_str);
}
void RenderEngine::DestroyPixelShader(uint32_t a_addr)
{
    m_backend->DestroyPixelShader(a_addr);
}