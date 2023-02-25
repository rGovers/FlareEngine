#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

#include <cstdint>

enum e_LightType : uint16_t
{
    LightType_Directional = 0,
    LightType_Point,
    LightType_Spot,
    LightType_End
};

struct DirectionalLightBuffer
{
    uint32_t TransformAddr;
    uint32_t RenderLayer;
    glm::vec4 Color;
    float Intensity;

    constexpr DirectionalLightBuffer(uint32_t a_transformAddr = -1, uint32_t a_renderLayer = 0b1, const glm::vec4& a_color = glm::vec4(1.0f), float a_intensity = 10.0f) :
        TransformAddr(a_transformAddr),
        RenderLayer(a_renderLayer),
        Color(a_color),
        Intensity(a_intensity)
    {

    }
};