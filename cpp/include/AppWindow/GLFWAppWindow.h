#pragma once

#include "AppWindow/AppWindow.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class Config;

class GLFWAppWindow : public AppWindow
{
private:
    GLFWwindow*    m_window;
   
    bool           m_shouldClose;

    double         m_time;
    double         m_prevTime;
    double         m_startTime;

    vk::SurfaceKHR m_surface;
    
protected:

public:
    GLFWAppWindow(Application* a_app, Config* a_config);
    virtual ~GLFWAppWindow();

    virtual bool ShouldClose() const;

    virtual double GetDelta() const;
    virtual double GetTime() const;

    virtual void Update();

    virtual glm::ivec2 GetSize() const;

    virtual bool IsHeadless() const
    {
        return false;
    }

    virtual std::vector<const char*> GetRequiredVulkanExtenions() const;
    virtual vk::SurfaceKHR GetSurface(const vk::Instance& a_instance);
};