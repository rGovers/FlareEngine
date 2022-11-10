#pragma once

#include <cstdint>

enum e_ShaderBufferType : uint16_t
{
    ShaderBufferType_Camera = 0,
    ShaderBufferType_Model = 1
};

enum e_ShaderSlot : uint16_t
{
    ShaderSlot_Vertex = 0,
    ShaderSlot_Pixel = 1,
    ShaderSlot_All = 2
};

struct ShaderBufferInput
{   
    uint32_t Slot;
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