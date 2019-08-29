#version 450

layout (binding = 0) uniform UBO 
{
	mat4 MVP;
} Buffer;

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec2 InUV;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec2 OutUV;

void main ()
{
	gl_Position = Buffer.MVP * vec4 (InPosition, 1);

	OutColor = vec4 (1, 1, 1, 1);
	OutUV = InUV;
}