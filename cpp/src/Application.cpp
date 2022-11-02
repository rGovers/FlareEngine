#include "Application.h"

#include "Config.h"
#include "Rendering/RenderEngine.h"
#include "RuntimeManager.h"

Application::Application()
{
    m_config = new Config("./config.xml");

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(1280, 720, m_config->GetApplicationName().begin(), nullptr, nullptr);

    m_runtime = new RuntimeManager();
    
    m_renderEngine = new RenderEngine(m_runtime, m_window, m_config);
}
Application::~Application()
{
    delete m_runtime;
    delete m_renderEngine;
    delete m_config;

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::Run(int32_t a_argc, char* a_argv[])
{
    m_runtime->Exec(a_argc, a_argv);

    m_startTime = glfwGetTime();
    m_prevTime = m_startTime;

    while (!glfwWindowShouldClose(m_window))
    {
        const double curTime = glfwGetTime();
        const double delta = curTime - m_prevTime;
        const double time = curTime - m_startTime;

        m_runtime->Update(delta, time);

        m_renderEngine->Update();

        glfwPollEvents();

        m_prevTime = curTime;
    }
}