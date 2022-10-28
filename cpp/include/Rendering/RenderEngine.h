#pragma once

#include <GLFW/glfw3.h>

enum e_RenderingEngine
{
    RenderingEngine_Null,
    RenderingEngine_Vulkan
};

class Config;
class RenderEngineBackend;

class RenderEngine
{
private:
    friend class VulkanRenderEngineBackend;

    Config*              m_config;

    RenderEngineBackend* m_backend;

    GLFWwindow*          m_window;

protected:

public:
    RenderEngine(GLFWwindow* a_window, Config* a_config);
    ~RenderEngine();

    void Update();
};