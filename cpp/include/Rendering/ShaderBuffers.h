#pragma once

#define GLM_FORCE_SWIZZLE 
#include <glm/glm.hpp>

struct CameraShaderBuffer
{
    alignas(16) glm::mat4 View;
    alignas(16) glm::mat4 Proj;
    alignas(16) glm::mat4 InvView;
    alignas(16) glm::mat4 InvProj;
    alignas(16) glm::mat4 ViewProj;
};

struct ModelShaderBuffer
{
    alignas(16) glm::mat4 Model;
    alignas(16) glm::mat4 InvModel;
};

struct TimeShaderBuffer
{
    // Packing because gpus are annoying with love of vectors
    alignas(16) glm::vec2 Time;
};