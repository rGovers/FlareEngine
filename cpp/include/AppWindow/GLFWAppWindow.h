#pragma once

#include "AppWindow/AppWindow.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

class Config;

class GLFWAppWindow : public AppWindow
{
private:
    GLFWwindow* m_window;

    double      m_time;
    double      m_prevTime;
    double      m_startTime;
protected:

public:
    GLFWAppWindow(Config* a_config);
    virtual ~GLFWAppWindow();

    virtual bool ShouldClose() const;

    virtual double GetDelta() const;
    virtual double GetTime() const;

    virtual void Update();
};