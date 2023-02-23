#include "Rendering/Vulkan/VulkanTextureSampler.h"

#include "FlareAssert.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

constexpr static vk::Filter GetFilterMode(e_TextureFilter a_filter)
{
    switch (a_filter)
    {
    case TextureFilter_Linear:
    {
        return vk::Filter::eLinear;
    }
    }

    return vk::Filter::eNearest;
} 

constexpr static vk::SamplerAddressMode GetAddressMode(e_TextureAddress a_address)
{
    switch (a_address)
    {
    case TextureAddress_MirroredRepeat:
    {
        return vk::SamplerAddressMode::eMirroredRepeat;
    }
    case TextureAddress_ClampToEdge:
    {
        return vk::SamplerAddressMode::eClampToEdge;
    }
    }

    return vk::SamplerAddressMode::eRepeat;
}

VulkanTextureSampler::VulkanTextureSampler(VulkanRenderEngineBackend* a_engine, const TextureSampler& a_sampler)
{
    TRACE("Creating texture sampler");
    m_engine = a_engine;

    const vk::Device device = m_engine->GetLogicalDevice();

    const vk::Filter filter = GetFilterMode(a_sampler.FilterMode);
    const vk::SamplerAddressMode address = GetAddressMode(a_sampler.AddressMode);

    const vk::SamplerCreateInfo samplerInfo = vk::SamplerCreateInfo
    (
        { },
        filter,
        filter,
        vk::SamplerMipmapMode::eNearest,
        address,
        address,
        address
    );

    FLARE_ASSERT_MSG_R(device.createSampler(&samplerInfo, nullptr, &m_sampler) == vk::Result::eSuccess, "Failed to create texture sampler");
}
VulkanTextureSampler::~VulkanTextureSampler()
{
    TRACE("Destroying texture sampler");
    const vk::Device device = m_engine->GetLogicalDevice();

    device.destroySampler(m_sampler);
}