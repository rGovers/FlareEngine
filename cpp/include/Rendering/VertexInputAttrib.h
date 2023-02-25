#pragma once

#include <cstdint>

enum e_VertexType : uint16_t
{
    VertexType_Null = UINT16_MAX,
    VertexType_Float = 0,
    VertexType_Int = 1,
    VertexType_UInt = 2
};

struct VertexInputAttrib
{
    uint16_t Location;
    e_VertexType Type;
    uint16_t Count;
    uint16_t Offset;

    constexpr VertexInputAttrib(uint16_t a_location = -1, e_VertexType a_type = VertexType_Null, uint16_t a_count = 0, uint16_t a_offset = 0) :
        Location(a_location),
        Type(a_type),
        Count(a_count),
        Offset(a_offset)
    {

    }

    constexpr bool operator ==(const VertexInputAttrib& a_other) const
    {
        return Location == a_other.Location && Type == a_other.Type && Count == a_other.Count && Offset == a_other.Offset;
    }
    constexpr bool operator !=(const VertexInputAttrib& a_other) const
    {
        return !(*this == a_other);
    }
};
