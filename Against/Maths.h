#pragma once

#define M_PI (float)3.14159265358979323846

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

void Vector3Normalize (Vector3 V, Vector3* Result);
void Vector4Normalize (Vector4 V, Vector4* Result);
void VectorMultiply (Vector3 V, float Scale, Vector3* Result);
void VectorDotProduct (Vector3 V1, Vector3 V2, float* Result);
void VectorCrossProduct (Vector3 V1, Vector3 V2, Vector3* Result);

void MatrixTranspose (Matrix4x4 M, Matrix4x4* Result);
void MatrixMultiplyMatrix (Matrix4x4 M1, Matrix4x4 M2, Matrix4x4* Result);
void MatrixMultiplyVector (Matrix4x4 M, Vector4 V, Vector4* Result);
void MatrixInverse (Matrix4x4 M, Matrix4x4 *Result);


void MatrixCreateModelEuler (Vector3 Translation, Vector3 Rotation, Vector3 Scale, Matrix4x4* Result);
void MatrixCreateModelQuaternion (Vector3 Translation, Vector4 Rotation, Vector3 Scale, Matrix4x4* Result);

void MatrixCreateProjection (float FOVDegrees, float AspectRatio, float NearPlane, float FarPlane, Matrix4x4* Result);
void MatrixCreateViewFromLookAt (Vector3 Eye, Vector3 Target, Vector3 Up, Matrix4x4* Result);
void MatrixCreateViewFromModel (Matrix4x4 M, Matrix4x4* Result);
void MatrixCreateViewFPS (Vector3 Eye, float Pitch, float Yaw, Matrix4x4* Result);

void MatrixTranslate (Matrix4x4 M, Vector3 Translation, Matrix4x4* Result);
void MatrixRotateEuler (Matrix4x4 M, Vector3 Rotation, Matrix4x4* Result);
void MatrixRotateQuaternion (Matrix4x4 M, Vector4 Rotation, Matrix4x4* Result);
void MatrixScale (Matrix4x4 M, Vector3 Scale, Matrix4x4* Result);

void DegreesToRadians (float Degrees, float* Radians);

Matrix4x4 MatrixGetIdentity ();
Matrix4x4 MatrixCreateF (float* Values);
Matrix4x4 MatrixCreateD (double* Values);