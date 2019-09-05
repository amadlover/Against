#version 450

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec2 InUV;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec2 OutUV;

void main ()
{
	gl_Position = vec4 (InPosition, 1);
	gl_Position.z = 1;
	gl_Position.w = 2;

	OutColor = vec4 (1, 1, 1, 1);
	OutUV = abs(vec2(0,1) - InUV);
}