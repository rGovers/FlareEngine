#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <cstdint>

class Config;
class RenderEngine;
class RuntimeManager;

class Application
{
private:
    GLFWwindow*     m_window;

    Config*         m_config;
    RuntimeManager* m_runtime;
    RenderEngine*   m_renderEngine;

    double          m_startTime;
    double          m_prevTime;

protected:

public:
    Application();
    ~Application();

    void Run(int32_t a_argc, char* a_argv[]);
};