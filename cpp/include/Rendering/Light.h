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
    uint32_t RenderLayer;
    uint32_t TransformAddr;
    glm::vec4 Color;
    float Intensity;
};