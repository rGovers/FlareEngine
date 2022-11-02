#pragma once

#include <cstdint>

#include "Viewport.h"

struct CameraBuffer
{
    Viewport View;
    uint32_t RenderLayer;
};
