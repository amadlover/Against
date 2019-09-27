#version 450

layout (binding = 1) uniform sampler2D ColorTexture;
layout (binding = 2) uniform FragUBO
{
	float Glow;
} Buffer;

layout (location = 0) in vec2 InUV;

layout (location = 0) out vec4 OutColor;

void main ()
{
	OutColor = texture (ColorTexture, InUV) * Buffer.Glow;
}