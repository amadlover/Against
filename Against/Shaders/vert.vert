#version 450

layout (binding = 0) uniform UBO 
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
	mat4 ModelMatrix;
} Buffer;

layout (location = 0) in vec3 InPosition;

layout (location = 0) out vec4 FragColor;

void main ()
{
	gl_Position = Buffer.ProjectionMatrix * Buffer.ViewMatrix * Buffer.ModelMatrix * vec4 (InPosition, 1);

	FragColor = vec4 (1, 1, 1, 1);
}