#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

struct Vertex
{
    glm::vec4 Position;
    glm::vec3 Normal;
    glm::vec4 Color;
    glm::vec2 TexCoords;
};
