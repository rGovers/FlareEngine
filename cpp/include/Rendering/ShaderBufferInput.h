#pragma once

#include <cstdint>

enum e_ShaderBufferType : uint16_t
{
    ShaderBufferType_Null = UINT16_MAX,
    ShaderBufferType_CameraBuffer = 0,
    ShaderBufferType_ModelBuffer = 1,
    ShaderBufferType_Texture = 2,
};

enum e_ShaderSlot : uint16_t
{
    ShaderSlot_Null = UINT16_MAX,
    ShaderSlot_Vertex = 0,
    ShaderSlot_Pixel = 1,
    ShaderSlot_All = 2
};

struct ShaderBufferInput
{   
    uint16_t Slot;
    e_ShaderBufferType BufferType;
    e_ShaderSlot ShaderSlot;

    constexpr ShaderBufferInput(uint16_t a_slot = -1, e_ShaderBufferType a_bufferType = ShaderBufferType_Null, e_ShaderSlot a_shaderSlot = ShaderSlot_Null) :
        Slot(a_slot),
        BufferType(a_bufferType),
        ShaderSlot(a_shaderSlot)
    {
        
    }

    constexpr bool operator ==(const ShaderBufferInput& a_other) const
    {
        return Slot == a_other.Slot && BufferType == a_other.BufferType && ShaderSlot == a_other.ShaderSlot;
    }
    constexpr bool operator !=(const ShaderBufferInput& a_other) const
    {
        return !(*this == a_other);
    }
};