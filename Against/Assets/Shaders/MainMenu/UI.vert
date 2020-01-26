#version 450

/*layout (set = 0, binding = 0) uniform VertUBO
{
	mat4 ModelViewProj;
	float Glow;
	int IsBackground;
} Buffer;*/

layout (push_constant) uniform VertUBO
{
	mat4 ModelViewProj;
	float Glow;
	int IsBackground;
} PushConsts;

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec2 InUV;

layout (location = 0) out vec2 OutUV;
layout (location = 1) out float OutGlow;

void main ()
{
	/*if (PushConsts.IsBackground == 1)
	{
		gl_Position = vec4 (InPosition, 1);
	}
	else 
	{*/
		gl_Position = PushConsts.ModelViewProj * vec4 (InPosition, 1);
	//}
	
	OutGlow = PushConsts.Glow;
	OutUV = InUV;
}