#pragma once

#include <cstdint>

enum e_VertexType : uint16_t
{
    VertexType_Float = 0,
    VertexType_Int = 1,
    VertexType_UInt = 2
};

struct VertexInputAttrib
{
    uint32_t Location;
    e_VertexType Type;
    uint32_t Count;
    uint32_t Offset;

    inline bool operator ==(const VertexInputAttrib& a_other) const
    {
        return Location == a_other.Location && Type == a_other.Type && Count == a_other.Count && Offset == a_other.Offset;
    }
    inline bool operator !=(const VertexInputAttrib& a_other) const
    {
        return !(*this == a_other);
    }
};
