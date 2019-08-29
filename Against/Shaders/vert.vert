#version 450

layout (binding = 0) uniform UBO 
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	mat4 ModelMatrix;
} Buffer;

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec2 InUV;

layout (location = 0) out vec4 OutColor;
layout (location = 1) out vec2 OutUV;

void main ()
{
	gl_Position = Buffer.ProjectionMatrix * Buffer.ViewMatrix * Buffer.ModelMatrix * vec4 (InPosition, 1);

	/*if (gl_VertexIndex == 0)
	{
		gl_Position = vec4(-1,-1,10,1);
	}
	
	if (gl_VertexIndex == 1)
	{
		gl_Position = vec4(-1,1,10,1);
	}
	
	if (gl_VertexIndex == 2)
	{
		gl_Position = vec4(1,1,10,1);
	}
	
	if (gl_VertexIndex == 3)
	{
		gl_Position = vec4(1,-1,10,1);
	}*/

	OutColor = vec4 (1, 1, 1, 1);
	OutUV = InUV;
}