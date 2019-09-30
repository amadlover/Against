#version 450

layout (set = 0, binding = 1) uniform sampler2D ColorTexture;
layout (set = 0, binding = 2) uniform FragUBO
{
	float Glow;
} Buffer;

layout (location = 0) in vec2 InUV;

layout (location = 0) out vec4 OutColor;

void main ()
{
	OutColor = vec4(1, 1, 1, 1) * Buffer.Glow;//texture (ColorTexture, InUV) + Buffer.Glow;
}