#version 450

layout(location = 0) in vec2 vUV;

layout(binding = 0) uniform sampler2D colorSampler;
layout(binding = 1) uniform sampler2D normalSampler;
layout(binding = 2) uniform sampler2D specSampler;
layout(binding = 3) uniform sampler2D dataSampler;

layout(location = 0) out vec4 fragColor;

void main()
{
    fragColor = vec4(texture(normalSampler, vUV).xyz, 1.0f);
}