#include "Application.h"

#include "Config.h"
#include "RuntimeManager.h"

Application::Application()
{
    m_config = new Config("./config.xml");
    m_runtime = new RuntimeManager();
}
Application::~Application()
{
    delete m_runtime;
}

void Application::Run(int32_t a_argc, char* a_argv[])
{
    m_runtime->Exec(a_argc, a_argv);

    while (true)
    {

    }
}