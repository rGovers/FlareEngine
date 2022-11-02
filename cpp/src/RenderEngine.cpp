#include "Rendering/RenderEngine.h"

#include <assert.h>

#include "Config.h"
#include "Rendering/SpirvTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "RuntimeManager.h"

RenderEngine::RenderEngine(RuntimeManager* a_runtime, GLFWwindow* a_window, Config* a_config)
{
    m_config = a_config;

    m_window = a_window;

    assert(m_config->GetRenderingEngine() != RenderingEngine_Null);

    switch (m_config->GetRenderingEngine())
    {
    case RenderingEngine_Vulkan:
    {
        m_backend = new VulkanRenderEngineBackend(a_runtime, this);

        break;
    }
    default:
    {
        printf("Failed to create RenderEngine \n");

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
