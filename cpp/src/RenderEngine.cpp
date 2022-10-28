#include "Rendering/RenderEngine.h"

#include <assert.h>

#include "Config.h"
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
}
RenderEngine::~RenderEngine()
{
    delete m_backend;
}

void RenderEngine::Update()
{
    m_backend->Update();
}