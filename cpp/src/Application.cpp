#include "Application.h"

#include "AppWindow/GLFWAppWindow.h"
#include "AppWindow/HeadlessAppWindow.h"
#include "Config.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Profiler.h"
#include "Rendering/RenderEngine.h"
#include "Runtime/RuntimeManager.h"
#include "Trace.h"

// Compiler why 
static void PlzNoReorder(RuntimeManager* a_runtime)
{
    delete a_runtime;
}

Application::Application(Config* a_config)
{
    m_config = a_config;

    if (a_config->IsHeadless())
    {
        m_appWindow = new HeadlessAppWindow();
    }
    else
    {
        m_appWindow = new GLFWAppWindow(a_config);
    }

    m_runtime = new RuntimeManager();

    Profiler::Init(m_runtime);
    Logger::InitRuntime(m_runtime);
    
    m_objectManager = new ObjectManager(m_runtime);

    m_renderEngine = new RenderEngine(m_runtime, m_objectManager, m_appWindow, m_config);
}
Application::~Application()
{
    TRACE("Disposing App");

    // This may seem odd but it seem that with the more recent changes GCC has decided to reorder the calls for some reason
    // Had to move the delete to a seperate computation unit to prevent reordering
    // Gonna guess a side effect of having execution outside of the scope of what GCC can predict at compile time
    // Do not know why C++ does not have a standard way to disable reordering
    // TLDR: Do not inline otherwise crash
    PlzNoReorder(m_runtime);
    delete m_renderEngine;
    delete m_objectManager;
    delete m_config;

    Profiler::Destroy();

    TRACE("Final Disposal");
    delete m_appWindow;
}

void Application::Run(int32_t a_argc, char* a_argv[])
{
    m_runtime->Exec(a_argc, a_argv);

    m_renderEngine->Start();

    while (!m_appWindow->ShouldClose())
    {
        Profiler::Start("Update Thread");
    
        {
            PROFILESTACK("Update");
            Profiler::StartFrame("Window Update");

            m_appWindow->Update();

            Profiler::StopFrame();

            m_runtime->Update(m_appWindow->GetDelta(), m_appWindow->GetTime());
        }

        Profiler::Stop();
    }

    m_renderEngine->Stop();
}