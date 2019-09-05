#pragma once

typedef struct _UV
{
	float u;
	float v;
} UV;

typedef struct _Vector2
{
	float x;
	float y;
} Vector2;

typedef struct _Color2
{
	float r;
	float g;
} Color2;

typedef struct _Vector3
{
	float x;
	float y;
	float z;
} Vector3;

typedef struct _Color3
{
	float r;
	float g;
	float b;
} Color3;

typedef struct _Vector4
{
	float x;
	float y;
	float z;
	float w;
} Vector4;

typedef struct _Color4
{
	float r;
	float g;
	float b;
	float a;
} Color4;

typedef struct _Matrix4x4
{
	float m[16];
} Matrix4x4;

#ifdef __cplusplus
extern "C" 
{
#endif
	void CreatePerspectiveProjectionMatrix (float FOV, float AspectRatio, float Near, float Far, Matrix4x4* Result);
	void CreateOrthographicProjectionMatrix (float Left, float Right, float Bottom, float Top, float Near, float Far, Matrix4x4* Result);
#ifdef __cplusplus
}
#endif