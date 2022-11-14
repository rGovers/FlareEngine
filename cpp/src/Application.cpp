#include "Application.h"

#include "AppWindow/GLFWAppWindow.h"
#include "AppWindow/HeadlessAppWindow.h"
#include "Config.h"
#include "Logger.h"
#include "ObjectManager.h"
#include "Rendering/RenderEngine.h"
#include "RuntimeManager.h"

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
    Logger::InitRuntime(m_runtime);
    
    m_objectManager = new ObjectManager(m_runtime);

    m_renderEngine = new RenderEngine(m_runtime, m_objectManager, m_appWindow, m_config);
}
Application::~Application()
{
    delete m_runtime;
    delete m_renderEngine;
    delete m_objectManager;
    delete m_config;

    delete m_appWindow;
}

void Application::Run(int32_t a_argc, char* a_argv[])
{
    m_runtime->Exec(a_argc, a_argv);

    while (!m_appWindow->ShouldClose())
    {
        m_appWindow->Update();

        m_runtime->Update(m_appWindow->GetDelta(), m_appWindow->GetTime());
 
        m_renderEngine->Update();
    }
}