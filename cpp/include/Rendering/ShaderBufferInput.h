#pragma once

#include <cstdint>

enum e_ShaderBufferType : uint16_t
{
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

    inline bool operator ==(const ShaderBufferInput& a_other) const
    {
        return Slot == a_other.Slot && BufferType == a_other.BufferType && ShaderSlot == a_other.ShaderSlot;
    }
    inline bool operator !=(const ShaderBufferInput& a_other) const
    {
        return !(*this == a_other);
    }
};