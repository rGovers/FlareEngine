#version 450

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in vec2 texCoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform CameraUBO
{
    mat4 View;
    mat4 Proj;
    mat4 InvView;
    mat4 InvProj;
    mat4 ViewProj;
} camUbo;

void main() 
{
    gl_Position = camUbo.ViewProj * position;

    fragColor = color;
}