#include "Rendering/RenderEngine.h"

#include <assert.h>

#include "Config.h"
#include "Logger.h"
#include "Rendering/SpirvTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "RuntimeManager.h"
#include "Trace.h"

RenderEngine::RenderEngine(RuntimeManager* a_runtime, ObjectManager* a_objectManager, AppWindow* a_window, Config* a_config)
{
    TRACE("Initializing Rendering");
    m_config = a_config;

    m_objectManager = a_objectManager;

    m_window = a_window;

    spirv_init();

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
        Logger::Error("Failed to create RenderEngine");

        assert(0);

        break;
    }
    }

    m_join = true;
}
RenderEngine::~RenderEngine()
{
    Stop();

    spirv_destroy();

    delete m_backend;
}

void RenderEngine::Start()
{
    TRACE("Starting Render Thread");
    m_shutdown = false;
    m_join = false;
    m_thread = std::thread(std::bind(&RenderEngine::Run, this));
}
void RenderEngine::Stop()
{
    if (m_join)
    {
        return;
    }

    TRACE("Stopping Render Thread");
    m_shutdown = true;
    while (!m_join) { }
    m_thread.join();
}

void RenderEngine::Run()
{
    while (!m_shutdown)
    {
        m_backend->Update();
    }
    
    m_join = true;
    TRACE("Render Thread joining");
}
void RenderEngine::Update()
{
    m_backend->Update();
}
