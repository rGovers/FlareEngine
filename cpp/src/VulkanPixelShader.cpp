#include "Rendering/Vulkan/VulkanPixelShader.h"

#include "Rendering/SpirvTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanPixelShader::VulkanPixelShader(RenderEngineBackend* a_engine, const std::vector<unsigned int>& a_data)
{
    m_engine = a_engine;

    const VulkanRenderEngineBackend* vEngine = (VulkanRenderEngineBackend*)m_engine;

    const vk::Device device = vEngine->GetLogicalDevice();

    const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
    (
        vk::ShaderModuleCreateFlags(),
        a_data.size() * sizeof(unsigned int),
        a_data.data()
    );

    if (device.createShaderModule(&createInfo, nullptr, &m_module) != vk::Result::eSuccess)
    {
        printf("Failed to create PixelShader");

        assert(0);
    }

    TRACE("Created PixelShader");
}
VulkanPixelShader::~VulkanPixelShader()
{
    const VulkanRenderEngineBackend* vEngine = (VulkanRenderEngineBackend*)m_engine;

    const vk::Device device = vEngine->GetLogicalDevice();

    device.destroyShaderModule(m_module);
}

VulkanPixelShader* VulkanPixelShader::CreateFromGLSL(RenderEngineBackend* a_engine, const std::string_view& a_str)
{
    const std::vector<unsigned int> spirv = spirv_fromGLSL(EShLangFragment, a_str);
    
    if (spirv.size() <= 0)
    {
        printf("Failed to generate Pixel Spirv");

        assert(0);
    }

    return new VulkanPixelShader(a_engine, spirv);
}