#include "AppWindow/GLFWAppWindow.h"

#include "Config.h"

GLFWAppWindow::GLFWAppWindow(Config* a_config) : AppWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(1280, 720, a_config->GetApplicationName().begin(), nullptr, nullptr);

    m_time = glfwGetTime();
    m_prevTime = m_time;
    m_startTime = m_time;
}
GLFWAppWindow::~GLFWAppWindow()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool GLFWAppWindow::ShouldClose() const
{
    return false;
}

double GLFWAppWindow::GetDelta() const
{
    return m_time - m_prevTime;
}
double GLFWAppWindow::GetTime() const
{
    return m_time - m_startTime;
}

void GLFWAppWindow::Update()
{
    glfwPollEvents();

    m_prevTime = m_time;
    m_time = glfwGetTime();
}