#pragma once

typedef struct _UV
{
	/*float u;
	float v;*/
	float uv[2];
} UV;

typedef struct _Vector2
{
	/*float x;
	float y;*/
	float v[2];
} Vector2;

typedef struct _Color2
{
	/*float r;
	float g;*/
	float c[2];
} Color2;

typedef struct _Vector3
{
	/*float x;
	float y;
	float z;*/
	float v[3];
} Vector3;

typedef struct _Color3
{
	/*float r;
	float g;
	float b;*/
	float c[3];
} Color3;

typedef struct _Vector4
{
	/*float x;
	float y;
	float z;
	float w;*/
	float v[4];
} Vector4;

typedef struct _Color4
{
	/*float r;
	float g;
	float b;
	float a;*/
	float c[4];
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