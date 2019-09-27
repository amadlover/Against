#version 450

layout (binding = 0) uniform VertUBO
{
	mat4 ModelViewProj;
} Buffer;

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec2 InUV;

layout (location = 0) out vec2 OutUV;

void main ()
{
	gl_Position = Buffer.ModelViewProj * vec4 (InPosition, 1);
	OutUV = InUV;
}