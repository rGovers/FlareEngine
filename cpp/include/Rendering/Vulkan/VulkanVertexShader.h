#pragma once

#include <string_view>

#include <vulkan/vulkan.hpp>

class RenderEngineBackend;

class VulkanVertexShader
{
private:
    RenderEngineBackend* m_engine = nullptr;

    vk::ShaderModule     m_module = nullptr;

    VulkanVertexShader();

protected:

public:
    VulkanVertexShader(RenderEngineBackend* a_engine, const std::vector<unsigned int>& a_data);
    ~VulkanVertexShader();

    static VulkanVertexShader* CreateFromGLSL(RenderEngineBackend* a_engine, const std::string_view& a_str);
};