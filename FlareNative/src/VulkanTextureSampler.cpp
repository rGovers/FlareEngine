#include "Rendering/Vulkan/VulkanTextureSampler.h"

#include "Flare/FlareAssert.h"
#include "Rendering/Vulkan/VulkanRenderEngineBackend.h"
#include "Trace.h"

constexpr static vk::Filter GetFilterMode(FlareBase::e_TextureFilter a_filter)
{
    switch (a_filter)
    {
    case FlareBase::TextureFilter_Linear:
    {
        return vk::Filter::eLinear;
    }
    }

    return vk::Filter::eNearest;
} 

constexpr static vk::SamplerAddressMode GetAddressMode(FlareBase::e_TextureAddress a_address)
{
    switch (a_address)
    {
    case FlareBase::TextureAddress_MirroredRepeat:
    {
        return vk::SamplerAddressMode::eMirroredRepeat;
    }
    case FlareBase::TextureAddress_ClampToEdge:
    {
        return vk::SamplerAddressMode::eClampToEdge;
    }
    }

    return vk::SamplerAddressMode::eRepeat;
}

VulkanTextureSampler::VulkanTextureSampler(VulkanRenderEngineBackend* a_engine, const FlareBase::TextureSampler& a_sampler)
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