#pragma once

#include <string_view>

#include <vulkan/vulkan.hpp>

class RenderEngineBackend;

class VulkanPixelShader
{
private:
    RenderEngineBackend* m_engine = nullptr;

    vk::ShaderModule     m_module = nullptr;

    VulkanPixelShader();

protected:

public:
    VulkanPixelShader(RenderEngineBackend* a_engine, const std::vector<unsigned int>& a_data);
    ~VulkanPixelShader();

    static VulkanPixelShader* CreateFromGLSL(RenderEngineBackend* a_engine, const std::string_view& a_str);
};