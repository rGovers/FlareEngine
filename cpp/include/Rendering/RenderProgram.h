#pragma once

#include <cstdint>

struct RenderProgram
{
    uint32_t VertexShader = -1;
    uint32_t PixelShader = -1;
    uint32_t RenderLayer;

    bool operator ==(const RenderProgram& a_other) const
    {
        return VertexShader == a_other.VertexShader && PixelShader == a_other.PixelShader && RenderLayer == a_other.RenderLayer;
    }
};