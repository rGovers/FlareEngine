#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <cstdint>

class Config;
class RuntimeManager;

class Application
{
private:
    GLFWwindow*     m_window;

    Config*         m_config;
    RuntimeManager* m_runtime;

protected:

public:
    Application();
    ~Application();

    void Run(int32_t a_argc, char* a_argv[]);
};