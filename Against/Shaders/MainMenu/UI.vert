#version 450

layout (set = 0, binding = 0) uniform VertUBO
{
	mat4 ModelViewProj;
	float Glow;
} Buffer;

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec2 InUV;

layout (location = 0) out vec2 OutUV;
layout (location = 1) out float OutGlow;

void main ()
{
	gl_Position = Buffer.ModelViewProj * vec4 (InPosition, 1);
	
	OutGlow = Buffer.Glow;
	OutUV = InUV;
}