#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

struct Viewport
{
    glm::vec2 Position;
    glm::vec2 Size;
    float MinDepth;
    float MaxDepth;
};
