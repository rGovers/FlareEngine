#pragma once

#include <cstdint>

#include "Rendering/ShaderBufferInput.h"
#include "Rendering/VertexInputAttrib.h"

struct RenderProgram
{
    uint32_t VertexShader = -1;
    uint32_t PixelShader = -1;
    uint32_t RenderLayer;
    uint16_t VertexStride;
    uint16_t VertexInputCount;
    VertexInputAttrib* VertexAttribs;
    uint16_t ShaderBufferInputCount;
    ShaderBufferInput* ShaderBufferInputs;

    bool operator ==(const RenderProgram& a_other) const
    {
        if (VertexShader != a_other.VertexShader || PixelShader != a_other.PixelShader || RenderLayer != a_other.RenderLayer)
        {
            return false;
        }

        if (VertexInputCount != a_other.VertexInputCount || ShaderBufferInputCount != a_other.ShaderBufferInputCount)
        {
            return false;
        }

        for (uint32_t i = 0; i < VertexInputCount; ++i)
        {
            if (VertexAttribs[i] != a_other.VertexAttribs[i])
            {
                return false;
            }
        }

        for (uint32_t i = 0; i < ShaderBufferInputCount; ++i)
        {
            if (ShaderBufferInputs[i] != a_other.ShaderBufferInputs[i])
            {
                return false;
            }
        }

        return true;
    }
    bool operator !=(const RenderProgram& a_other) const
    {
        return !(*this == a_other);
    }
};