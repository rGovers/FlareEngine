#pragma once

#include <GLFW/glfw3.h>
#include <string_view>

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

    uint32_t GenerateVertexShaderAddr(const std::string_view& a_str);
    void DestroyVertexShader(uint32_t a_addr);

    uint32_t GeneratePixelShaderAddr(const std::string_view& a_str);
    void DestroyPixelShader(uint32_t a_addr);
};