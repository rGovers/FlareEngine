#pragma once

#include <cstdint>

#include "Viewport.h"

#include <glm/ext/matrix_clip_space.hpp>

struct CameraBuffer
{
    uint32_t TransformAddr;
    Viewport View;
    uint32_t RenderLayer;
    float FOV;
    float Near;
    float Far;

    glm::mat4 ToProjection(const glm::vec2& a_screenSize) const
    {
        return glm::perspective(FOV, a_screenSize.x / a_screenSize.y, Near, Far);
    }
};
