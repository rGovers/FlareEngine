#include "Application.h"

#include "Config.h"
#include "RuntimeManager.h"

Application::Application()
{
    m_config = new Config("./config.xml");
    m_runtime = new RuntimeManager();

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_window = glfwCreateWindow(1280, 720, "FlashFire", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    printf("extension count: %d \n", extensionCount);
}
Application::~Application()
{
    delete m_runtime;
    delete m_config;

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Application::Run(int32_t a_argc, char* a_argv[])
{
    m_runtime->Exec(a_argc, a_argv);

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }
}