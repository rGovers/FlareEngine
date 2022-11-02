#include "Rendering/Vulkan/VulkanVertexShader.h"

#include "Rendering/SpirvTools.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

VulkanVertexShader::VulkanVertexShader(VulkanRenderEngineBackend* a_engine, const std::vector<unsigned int>& a_data) : VulkanShader(a_engine)
{
    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::ShaderModuleCreateInfo createInfo = vk::ShaderModuleCreateInfo
    (
        vk::ShaderModuleCreateFlags(),
        a_data.size() * sizeof(unsigned int),
        (uint32_t*)a_data.data()
    );

    if (device.createShaderModule(&createInfo, nullptr, &m_module) != vk::Result::eSuccess)
    {
        printf("Failed to create VertexShader");

        assert(0);
    }

    TRACE("Created VertexShader");
}
VulkanVertexShader::~VulkanVertexShader()
{
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroyShaderModule(m_module);
}

VulkanVertexShader* VulkanVertexShader::CreateFromGLSL(VulkanRenderEngineBackend* a_engine, const std::string_view& a_str)
{
    const std::vector<unsigned int> spirv = spirv_fromGLSL(EShLangVertex, a_str);
    
    if (spirv.size() <= 0)
    {
        printf("Failed to generate Vertex Spirv");

        assert(0);
    }

    return new VulkanVertexShader(a_engine, spirv);
}