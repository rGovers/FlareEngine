#include "AppWindow/GLFWAppWindow.h"

#include "Config.h"

GLFWAppWindow::GLFWAppWindow(Config* a_config) : AppWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(1280, 720, a_config->GetApplicationName().data(), nullptr, nullptr);

    m_time = glfwGetTime();
    m_prevTime = m_time;
    m_startTime = m_time;

    m_surface = nullptr;

    m_shouldClose = false;
}
GLFWAppWindow::~GLFWAppWindow()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool GLFWAppWindow::ShouldClose() const
{
    return m_shouldClose;
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

    m_shouldClose = glfwWindowShouldClose(m_window);
}

glm::ivec2 GLFWAppWindow::GetSize() const
{
    glm::ivec2 winSize;
    glfwGetWindowSize(m_window, &winSize.x, &winSize.y);

    return winSize;
}

vk::SurfaceKHR GLFWAppWindow::GetSurface(const vk::Instance& a_instance) 
{
    if (m_surface == vk::SurfaceKHR(nullptr))
    {
        VkSurfaceKHR tempSurf;
        glfwCreateWindowSurface(a_instance, m_window, nullptr, &tempSurf);
        m_surface = vk::SurfaceKHR(tempSurf);
    }
    
    return m_surface;
}
std::vector<const char*> GLFWAppWindow::GetRequiredVulkanExtenions() const
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}