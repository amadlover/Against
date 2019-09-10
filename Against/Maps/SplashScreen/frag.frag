#version 450

layout (binding = 0) uniform sampler2D SamplerColor;

layout (location = 0) in vec3 InColor;
layout (location = 1) in vec2 InUV;

layout (location = 0) out vec4 OutColor;

void main ()
{
	vec4 Color = texture (SamplerColor, InUV);
	OutColor = Color;
}