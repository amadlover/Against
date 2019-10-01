#version 450

layout (set = 1, binding = 0) uniform sampler2D ColorTexture;

layout (location = 0) in vec2 InUV;
layout (location = 1) in float InGlow;

layout (location = 0) out vec4 OutColor;

void main ()
{
	OutColor = vec4(1, 1, 1, 1) * InGlow;//texture (ColorTexture, InUV) + Buffer.Glow;
}