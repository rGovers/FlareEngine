#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <cstdint>

class Config;
class ObjectManager;
class RenderEngine;
class RuntimeManager;

class Application
{
private:
    GLFWwindow*     m_window;

    Config*         m_config;
    ObjectManager*  m_objectManager;
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