#include "Maths.h"

#include <math.h>

#include <Windows.h>

void DegreesToRadians (float Degrees, float* Radians)
{
	*Radians = Degrees * (M_PI / 180.f);
}

Matrix4x4 MatrixGetIdentity ()
{
	Matrix4x4 M;
	memset (&M, 0, sizeof (Matrix4x4));

	M.m[0] = 1;
	M.m[5] = 1;
	M.m[10] = 1;
	M.m[15] = 1;

	return M;
}

Matrix4x4 MatrixCreateF (float* Values)
{
	Matrix4x4 M;

	memcpy_s (M.m, sizeof (float) * 16, Values, sizeof (float) * 16);

	return M;
}

Matrix4x4 MatrixCreateD (double* Values)
{
	Matrix4x4 M;

	for (size_t i = 0; i < 16; i++)
	{
		M.m[i] = (float)Values[i];
	}

	return M;
}

void Vector3Normalize (Vector3 V, Vector3* Result)
{
	float Length = sqrtf (V.x * V.x) + (V.y* V.y) + (V.z * V.z);

	Result->x = V.x / Length;
	Result->y = V.y / Length;
	Result->z = V.z / Length;
}

void Vector4Normalize (Vector4 V, Vector4* Result)
{
	float Length = sqrtf ((V.x * V.x) + (V.y* V.y) + (V.z * V.z) + (V.w * V.w));

	Result->x = V.x / Length;
	Result->y = V.y / Length;
	Result->z = V.z / Length;
	Result->w = V.w / Length;
}

void VectorMultiply (Vector3 V, float Scale, Vector3* Result)
{
	Result->x = V.x * Scale;
	Result->y = V.y * Scale;
	Result->z = V.z * Scale;
}

void VectorDotProduct (Vector3 V1, Vector3 V2, float* Result)
{
	*Result = (V1.x * V2.x) + (V1.y * V2.y) + (V1.z * V2.z);
}

void VectorCrossProduct (Vector3 V1, Vector3 V2, Vector3* Result)
{
	Result->x = (V1.y * V2.z) - (V1.z * V2.y);
	Result->y = (V1.z * V2.x) - (V1.x * V2.z);
	Result->z = (V1.x * V2.y) - (V1.y * V2.x);
}

void MatrixCreatePerspectiveProjection (float FOVDegrees, float AspectRatio, float NearPlane, float FarPlane, Matrix4x4* Result)
{
	memset (Result, 0, sizeof (Matrix4x4));

	float FOVRadians;
	DegreesToRadians (FOVDegrees * 0.5f, &FOVRadians);

	float Scale = 1 / tanf (FOVRadians);

	Result->m[0] = Scale / AspectRatio;
	Result->m[5] = Scale;
	Result->m[10] = -FarPlane / (FarPlane - NearPlane);
	Result->m[14] = -FarPlane * NearPlane / (FarPlane - NearPlane);
	Result->m[11] = -1;
	Result->m[15] = 0;
}

void MatrixCreateViewFromLookAt (Vector3 Eye, Vector3 Target, Vector3 Up, Matrix4x4* Result)
{
	memset (Result, 0, sizeof (Matrix4x4));

	Vector3 Forward;
	Forward.x = Target.x - Eye.x;
	Forward.y = Target.y - Eye.y;
	Forward.z = Target.z - Eye.z;

	Vector3Normalize (Forward, &Forward);

	float UpDotForward;
	VectorDotProduct (Up, Forward, &UpDotForward);

	VectorMultiply (Up, UpDotForward, &Up);

	Vector3 Right;
	VectorCrossProduct (Forward, Up, &Right);

	Result->m[0] = Forward.x; Result->m[1] = Forward.x; Result->m[2] = Forward.x; Result->m[3] = 0;
	Result->m[4] = Up.x; Result->m[5] = Up.y; Result->m[6] = Up.z; Result->m[7] = 0;
	Result->m[8] = Right.x; Result->m[9] = Right.y; Result->m[10] = Right.z; Result->m[11] = 0;
	Result->m[12] = -Eye.x; Result->m[13] = -Eye.y; Result->m[14] = -Eye.z; Result->m[15] = 1;
}

void MatrixTranspose (Matrix4x4 M, Matrix4x4* Result)
{
	Result->m[0] = M.m[0];
	Result->m[1] = M.m[4];
	Result->m[2] = M.m[8];
	Result->m[3] = M.m[12];
	Result->m[4] = M.m[1];
	Result->m[5] = M.m[5];
	Result->m[6] = M.m[9];
	Result->m[7] = M.m[13];
	Result->m[8] = M.m[2];
	Result->m[9] = M.m[6];
	Result->m[10] = M.m[10];
	Result->m[11] = M.m[14];
	Result->m[12] = M.m[3];
	Result->m[13] = M.m[7];
	Result->m[14] = M.m[11];
	Result->m[15] = M.m[15];
}

void MatrixMultiplyMatrix (Matrix4x4 M1, Matrix4x4 M2, Matrix4x4* Result)
{
	Result->m[0] = (M1.m[0] * M2.m[0]) + (M1.m[4] * M2.m[1]) + (M1.m[8] * M2.m[2]) + (M1.m[12] * M2.m[3]);
	Result->m[1] = (M1.m[1] * M2.m[0]) + (M1.m[5] * M2.m[1]) + (M1.m[9] * M2.m[2]) + (M1.m[13] * M2.m[3]);
	Result->m[2] = (M1.m[2] * M2.m[0]) + (M1.m[6] * M2.m[1]) + (M1.m[10] * M2.m[2]) + (M1.m[14] * M2.m[3]);
	Result->m[3] = (M1.m[3] * M2.m[0]) + (M1.m[7] * M2.m[1]) + (M1.m[11] * M2.m[2]) + (M1.m[15] * M2.m[3]);
	Result->m[4] = (M1.m[0] * M2.m[4]) + (M1.m[4] * M2.m[5]) + (M1.m[8] * M2.m[6]) + (M1.m[12] * M2.m[7]);
	Result->m[5] = (M1.m[1] * M2.m[4]) + (M1.m[5] * M2.m[5]) + (M1.m[9] * M2.m[6]) + (M1.m[13] * M2.m[7]);
	Result->m[6] = (M1.m[2] * M2.m[4]) + (M1.m[6] * M2.m[5]) + (M1.m[10] * M2.m[6]) + (M1.m[14] * M2.m[7]);
	Result->m[7] = (M1.m[3] * M2.m[4]) + (M1.m[7] * M2.m[5]) + (M1.m[11] * M2.m[6]) + (M1.m[15] * M2.m[7]);
	Result->m[8] = (M1.m[0] * M2.m[8]) + (M1.m[4] * M2.m[9]) + (M1.m[8] * M2.m[10]) + (M1.m[12] * M2.m[11]);
	Result->m[9] = (M1.m[1] * M2.m[8]) + (M1.m[5] * M2.m[9]) + (M1.m[9] * M2.m[10]) + (M1.m[13] * M2.m[11]);
	Result->m[10] = (M1.m[2] * M2.m[8]) + (M1.m[6] * M2.m[9]) + (M1.m[10] * M2.m[10]) + (M1.m[14] * M2.m[11]);
	Result->m[11] = (M1.m[3] * M2.m[8]) + (M1.m[7] * M2.m[9]) + (M1.m[11] * M2.m[10]) + (M1.m[15] * M2.m[11]);
	Result->m[12] = (M1.m[0] * M2.m[12]) + (M1.m[4] * M2.m[13]) + (M1.m[8] * M2.m[14]) + (M1.m[12] * M2.m[15]);
	Result->m[13] = (M1.m[1] * M2.m[12]) + (M1.m[5] * M2.m[13]) + (M1.m[9] * M2.m[14]) + (M1.m[13] * M2.m[15]);
	Result->m[14] = (M1.m[2] * M2.m[12]) + (M1.m[6] * M2.m[13]) + (M1.m[10] * M2.m[14]) + (M1.m[14] * M2.m[15]);
	Result->m[15] = (M1.m[3] * M2.m[12]) + (M1.m[7] * M2.m[13]) + (M1.m[11] * M2.m[14]) + (M1.m[15] * M2.m[15]);
}

void MatrixInverse (Matrix4x4 M, Matrix4x4* Result)
{
	Matrix4x4 Inv = MatrixGetIdentity ();

	Inv.m[0] = M.m[5] * M.m[10] * M.m[15] -
		M.m[5] * M.m[11] * M.m[14] -
		M.m[9] * M.m[6] * M.m[15] +
		M.m[9] * M.m[7] * M.m[14] +
		M.m[13] * M.m[6] * M.m[11] -
		M.m[13] * M.m[7] * M.m[10];

	Inv.m[4] = -M.m[4] * M.m[10] * M.m[15] +
		M.m[4] * M.m[11] * M.m[14] +
		M.m[8] * M.m[6] * M.m[15] -
		M.m[8] * M.m[7] * M.m[14] -
		M.m[12] * M.m[6] * M.m[11] +
		M.m[12] * M.m[7] * M.m[10];

	Inv.m[8] = M.m[4] * M.m[9] * M.m[15] -
		M.m[4] * M.m[11] * M.m[13] -
		M.m[8] * M.m[5] * M.m[15] +
		M.m[8] * M.m[7] * M.m[13] +
		M.m[12] * M.m[5] * M.m[11] -
		M.m[12] * M.m[7] * M.m[9];

	Inv.m[12] = -M.m[4] * M.m[9] * M.m[14] +
		M.m[4] * M.m[10] * M.m[13] +
		M.m[8] * M.m[5] * M.m[14] -
		M.m[8] * M.m[6] * M.m[13] -
		M.m[12] * M.m[5] * M.m[10] +
		M.m[12] * M.m[6] * M.m[9];

	Inv.m[1] = -M.m[1] * M.m[10] * M.m[15] +
		M.m[1] * M.m[11] * M.m[14] +
		M.m[9] * M.m[2] * M.m[15] -
		M.m[9] * M.m[3] * M.m[14] -
		M.m[13] * M.m[2] * M.m[11] +
		M.m[13] * M.m[3] * M.m[10];

	Inv.m[5] = M.m[0] * M.m[10] * M.m[15] -
		M.m[0] * M.m[11] * M.m[14] -
		M.m[8] * M.m[2] * M.m[15] +
		M.m[8] * M.m[3] * M.m[14] +
		M.m[12] * M.m[2] * M.m[11] -
		M.m[12] * M.m[3] * M.m[10];

	Inv.m[9] = -M.m[0] * M.m[9] * M.m[15] +
		M.m[0] * M.m[11] * M.m[13] +
		M.m[8] * M.m[1] * M.m[15] -
		M.m[8] * M.m[3] * M.m[13] -
		M.m[12] * M.m[1] * M.m[11] +
		M.m[12] * M.m[3] * M.m[9];

	Inv.m[13] = M.m[0] * M.m[9] * M.m[14] -
		M.m[0] * M.m[10] * M.m[13] -
		M.m[8] * M.m[1] * M.m[14] +
		M.m[8] * M.m[2] * M.m[13] +
		M.m[12] * M.m[1] * M.m[10] -
		M.m[12] * M.m[2] * M.m[9];

	Inv.m[2] = M.m[1] * M.m[6] * M.m[15] -
		M.m[1] * M.m[7] * M.m[14] -
		M.m[5] * M.m[2] * M.m[15] +
		M.m[5] * M.m[3] * M.m[14] +
		M.m[13] * M.m[2] * M.m[7] -
		M.m[13] * M.m[3] * M.m[6];

	Inv.m[6] = -M.m[0] * M.m[6] * M.m[15] +
		M.m[0] * M.m[7] * M.m[14] +
		M.m[4] * M.m[2] * M.m[15] -
		M.m[4] * M.m[3] * M.m[14] -
		M.m[12] * M.m[2] * M.m[7] +
		M.m[12] * M.m[3] * M.m[6];

	Inv.m[10] = M.m[0] * M.m[5] * M.m[15] -
		M.m[0] * M.m[7] * M.m[13] -
		M.m[4] * M.m[1] * M.m[15] +
		M.m[4] * M.m[3] * M.m[13] +
		M.m[12] * M.m[1] * M.m[7] -
		M.m[12] * M.m[3] * M.m[5];

	Inv.m[14] = -M.m[0] * M.m[5] * M.m[14] +
		M.m[0] * M.m[6] * M.m[13] +
		M.m[4] * M.m[1] * M.m[14] -
		M.m[4] * M.m[2] * M.m[13] -
		M.m[12] * M.m[1] * M.m[6] +
		M.m[12] * M.m[2] * M.m[5];

	Inv.m[3] = -M.m[1] * M.m[6] * M.m[11] +
		M.m[1] * M.m[7] * M.m[10] +
		M.m[5] * M.m[2] * M.m[11] -
		M.m[5] * M.m[3] * M.m[10] -
		M.m[9] * M.m[2] * M.m[7] +
		M.m[9] * M.m[3] * M.m[6];

	Inv.m[7] = M.m[0] * M.m[6] * M.m[11] -
		M.m[0] * M.m[7] * M.m[10] -
		M.m[4] * M.m[2] * M.m[11] +
		M.m[4] * M.m[3] * M.m[10] +
		M.m[8] * M.m[2] * M.m[7] -
		M.m[8] * M.m[3] * M.m[6];

	Inv.m[11] = -M.m[0] * M.m[5] * M.m[11] +
		M.m[0] * M.m[7] * M.m[9] +
		M.m[4] * M.m[1] * M.m[11] -
		M.m[4] * M.m[3] * M.m[9] -
		M.m[8] * M.m[1] * M.m[7] +
		M.m[8] * M.m[3] * M.m[5];

	Inv.m[15] = M.m[0] * M.m[5] * M.m[10] -
		M.m[0] * M.m[6] * M.m[9] -
		M.m[4] * M.m[1] * M.m[10] +
		M.m[4] * M.m[2] * M.m[9] +
		M.m[8] * M.m[1] * M.m[6] -
		M.m[8] * M.m[2] * M.m[5];

	float Determinant = M.m[0] * Inv.m[0] + M.m[1] * Inv.m[4] + M.m[2] * Inv.m[8] + M.m[3] * Inv.m[12];

	if (Determinant)
	{
		Determinant = 1.f/ Determinant;

		for (short i = 0; i < 16; i++)
		{
			Result->m[i] = Inv.m[i] / Determinant;
		}
	}
}

void MatrixMultiplyVector (Matrix4x4 M, Vector4 V, Vector4* Result)
{
	Matrix4x4 VM ;
	VM.m[0] = V.x;
	VM.m[1] = V.y;
	VM.m[2] = V.z;
	VM.m[3] = V.w;
	
	Matrix4x4 RM ;

	MatrixMultiplyMatrix (M, VM, &RM);

	Result->x = RM.m[0];
	Result->y = RM.m[1];
	Result->z = RM.m[2];
	Result->w = RM.m[3];
}

void MatrixCreateModelFromEuler (Vector3 Translation, Vector3 Rotation, Vector3 Scale, Matrix4x4* Result)
{
	memset (Result, 0, sizeof (Result));

	Matrix4x4 TranslationMatrix = MatrixGetIdentity ();

	TranslationMatrix.m[12] = Translation.x;
	TranslationMatrix.m[13] = Translation.y;
	TranslationMatrix.m[14] = Translation.z;

	Vector3 RotationRadians ;
	DegreesToRadians (Rotation.x, &RotationRadians.x); DegreesToRadians (Rotation.y, &RotationRadians.y); DegreesToRadians (Rotation.z, &RotationRadians.z);

	float CosRotX = cosf (RotationRadians.x); float SinRotX = sinf (RotationRadians.x);

	Matrix4x4 RotationMatrixX = MatrixGetIdentity ();

	RotationMatrixX.m[0] = 1; RotationMatrixX.m[1] = 0; RotationMatrixX.m[2] = 0;
	RotationMatrixX.m[4] = 0; RotationMatrixX.m[5] = CosRotX; RotationMatrixX.m[6] = SinRotX;
	RotationMatrixX.m[8] = 0; RotationMatrixX.m[9] = -SinRotX; RotationMatrixX.m[10] = CosRotX;

	float CosRotY = cosf (RotationRadians.y); float SinRotY = sinf (RotationRadians.y);

	Matrix4x4 RotationMatrixY = MatrixGetIdentity ();

	RotationMatrixY.m[0] = CosRotY; RotationMatrixY.m[1] = 0; RotationMatrixY.m[2] = -SinRotY;
	RotationMatrixY.m[4] = 0; RotationMatrixY.m[5] = 1; RotationMatrixY.m[6] = 0;
	RotationMatrixY.m[8] = -SinRotY; RotationMatrixY.m[9] = 0; RotationMatrixY.m[10] = CosRotY;

	float CosRotZ = cosf (RotationRadians.z); float SinRotZ = sinf (RotationRadians.z);

	Matrix4x4 RotationMatrixZ = MatrixGetIdentity ();

	RotationMatrixZ.m[0] = CosRotZ; RotationMatrixZ.m[1] = SinRotZ; RotationMatrixZ.m[2] = 0;
	RotationMatrixZ.m[4] = -SinRotZ; RotationMatrixZ.m[5] = CosRotZ; RotationMatrixZ.m[6] = 0;
	RotationMatrixZ.m[8] = 0; RotationMatrixZ.m[9] = 0; RotationMatrixZ.m[10] = 1;

	Matrix4x4 RotationMatrixYX = MatrixGetIdentity ();
	MatrixMultiplyMatrix (RotationMatrixY, RotationMatrixX, &RotationMatrixYX);

	Matrix4x4 RotationMatrixYXZ = MatrixGetIdentity ();
	MatrixMultiplyMatrix (RotationMatrixYX, RotationMatrixZ, &RotationMatrixYXZ);

	Matrix4x4 ScaleMatrix = MatrixGetIdentity ();

	ScaleMatrix.m[0] = Scale.x;
	ScaleMatrix.m[5] = Scale.y;
	ScaleMatrix.m[10] = Scale.z;

	Matrix4x4 TransRotMatrix = MatrixGetIdentity ();

	MatrixMultiplyMatrix (TranslationMatrix, RotationMatrixYXZ, &TransRotMatrix);
	MatrixMultiplyMatrix (TransRotMatrix, ScaleMatrix, Result);
}

void MatrixCreateModelFromQuaternion (Vector3 Translation, Vector4 Rotation, Vector3 Scale, Matrix4x4* Result)
{
	memset (Result, 0, sizeof (Result));

	Matrix4x4 TranslationMatrix = MatrixGetIdentity ();

	TranslationMatrix.m[12] = Translation.x;
	TranslationMatrix.m[13] = Translation.y;
	TranslationMatrix.m[14] = Translation.z;

	Vector4 Normalized = { 0,0,0 };
	Vector4Normalize (Rotation, &Normalized);

	Matrix4x4 RotationMatrix;

	RotationMatrix.m[0] = 1.0f - 2.0f * Normalized.y * Normalized.y - 2.0f * Normalized.z * Normalized.z;
	RotationMatrix.m[1] = 2.0f * Normalized.x * Normalized.y - 2.0f * Normalized.z * Normalized.w;
	RotationMatrix.m[2] = 2.0f * Normalized.x * Normalized.z + 2.0f * Normalized.y * Normalized.w;
	RotationMatrix.m[3] = 0;

	RotationMatrix.m[4] = 2.0f * Normalized.x * Normalized.y + 2.0f * Normalized.z * Normalized.w;
	RotationMatrix.m[5] = 1.0f - 2.0f * Normalized.x * Normalized.x - 2.0f * Normalized.z * Normalized.z;
	RotationMatrix.m[6] = 2.0f * Normalized.y * Normalized.z - 2.0f * Normalized.x * Normalized.w;
	RotationMatrix.m[7] = 0;

	RotationMatrix.m[8] = 2.0f * Normalized.x * Normalized.z - 2.0f * Normalized.y * Normalized.w;
	RotationMatrix.m[9] = 2.0f * Normalized.y * Normalized.z + 2.0f * Normalized.x * Normalized.w;
	RotationMatrix.m[10] = 1.0f - 2.0f * Normalized.x * Normalized.x - 2.0f * Normalized.y * Normalized.y;
	RotationMatrix.m[11] = 0;

	RotationMatrix.m[12] = 0;
	RotationMatrix.m[13] = 0;
	RotationMatrix.m[14] = 0;
	RotationMatrix.m[15] = 1;

	Matrix4x4 ScaleMatrix = MatrixGetIdentity ();

	ScaleMatrix.m[0] = Scale.x;
	ScaleMatrix.m[5] = Scale.y;
	ScaleMatrix.m[10] = Scale.z;

	Matrix4x4 TransRotMatrix = MatrixGetIdentity ();

	MatrixMultiplyMatrix (TranslationMatrix, RotationMatrix, &TransRotMatrix);
	MatrixMultiplyMatrix (TransRotMatrix, ScaleMatrix, Result);
}

void MatrixCreateViewFPS (Vector3 Eye, float Pitch, float Yaw, Matrix4x4* Result)
{
	memset (Result, 0, sizeof (Matrix4x4));

	float PitchRadians;
	DegreesToRadians (Pitch, &PitchRadians);

	float YawRadians;
	DegreesToRadians (Yaw, &YawRadians);

	float CosPitch = cosf (PitchRadians);
	float SinPitch = sinf (PitchRadians);

	float CosYaw = cosf (YawRadians);
	float SinYaw = sinf (YawRadians);

	Vector3 XAxis ;
	XAxis.x = CosYaw;
	XAxis.y = 0;
	XAxis.z = -SinYaw;

	Vector3 YAxis ;
	YAxis.x = SinYaw * SinPitch;
	YAxis.y = CosPitch;
	YAxis.z = CosYaw * SinPitch;

	Vector3 ZAxis ;
	ZAxis.x = SinYaw * CosPitch;
	ZAxis.y = -SinPitch;
	ZAxis.z = CosPitch * CosYaw;

	float XAxisDotEye, YAxisDotEye, ZAxisDotEye;
	VectorDotProduct (XAxis, Eye, &XAxisDotEye);
	VectorDotProduct (YAxis, Eye, &YAxisDotEye);
	VectorDotProduct (ZAxis, Eye, &ZAxisDotEye);

	Result->m[0] = XAxis.x; Result->m[1] = YAxis.x; Result->m[2] = ZAxis.x; Result->m[3] = 0;
	Result->m[4] = XAxis.y; Result->m[5] = YAxis.y; Result->m[6] = ZAxis.y; Result->m[7] = 0;
	Result->m[8] = XAxis.z; Result->m[9] = YAxis.z; Result->m[10] = ZAxis.z; Result->m[11] = 0;
	Result->m[12] = -XAxisDotEye; Result->m[13] = -YAxisDotEye; Result->m[14] = -ZAxisDotEye; Result->m[15] = 1;
}

void MatrixCreateViewFromModel (Matrix4x4 M, Matrix4x4* Result)
{
	memset (Result, 0, sizeof (Matrix4x4));

}

void MatrixCreateOrthographicProjection (float Left, float Right, float Bottom, float Top, float Near, float Far, Matrix4x4* Result)
{
	memset (Result, 0, sizeof (Matrix4x4));

	Result->m[0] = 2.f / (Right - Left);
	Result->m[5] = 2.f / (Bottom - Top);
	Result->m[10] = -2.f / (Far - Near);

	Result->m[12] = -(Right + Left) / (Right - Left);
	Result->m[13] = -(Bottom + Top) / (Bottom - Top);
	Result->m[14] = -(Near + Far) / (Near - Far);
	Result->m[15] = 1.f;
}

void MatrixTranslate (Matrix4x4 M, Vector3 Translation, Matrix4x4* Result)
{
	Matrix4x4 TranslationMatrix = MatrixGetIdentity ();

	TranslationMatrix.m[12] = Translation.x;
	TranslationMatrix.m[13] = Translation.y;
	TranslationMatrix.m[14] = Translation.z;

	MatrixMultiplyMatrix (M, TranslationMatrix, Result);
}

void MatrixRotateEuler (Matrix4x4 M, Vector3 Rotation, Matrix4x4* Result)
{
	Vector3 RotationRadians ;
	DegreesToRadians (Rotation.x, &RotationRadians.x); DegreesToRadians (Rotation.y, &RotationRadians.y); DegreesToRadians (Rotation.z, &RotationRadians.z);

	float CosRotX = cosf (RotationRadians.x); float SinRotX = sinf (RotationRadians.x);

	Matrix4x4 RotationMatrixX = MatrixGetIdentity ();

	RotationMatrixX.m[0] = 1; RotationMatrixX.m[1] = 0; RotationMatrixX.m[2] = 0;
	RotationMatrixX.m[4] = 0; RotationMatrixX.m[5] = CosRotX; RotationMatrixX.m[6] = SinRotX;
	RotationMatrixX.m[8] = 0; RotationMatrixX.m[9] = -SinRotX; RotationMatrixX.m[10] = CosRotX;

	float CosRotY = cosf (RotationRadians.y); float SinRotY = sinf (RotationRadians.y);

	Matrix4x4 RotationMatrixY = MatrixGetIdentity ();

	RotationMatrixY.m[0] = CosRotY; RotationMatrixY.m[1] = 0; RotationMatrixY.m[2] = -SinRotY;
	RotationMatrixY.m[4] = 0; RotationMatrixY.m[5] = 1; RotationMatrixY.m[6] = 0;
	RotationMatrixY.m[8] = -SinRotY; RotationMatrixY.m[9] = 0; RotationMatrixY.m[10] = CosRotY;

	float CosRotZ = cosf (RotationRadians.z); float SinRotZ = sinf (RotationRadians.z);

	Matrix4x4 RotationMatrixZ = MatrixGetIdentity ();

	RotationMatrixZ.m[0] = CosRotZ; RotationMatrixZ.m[1] = SinRotZ; RotationMatrixZ.m[2] = 0;
	RotationMatrixZ.m[4] = -SinRotZ; RotationMatrixZ.m[5] = CosRotZ; RotationMatrixZ.m[6] = 0;
	RotationMatrixZ.m[8] = 0; RotationMatrixZ.m[9] = 0; RotationMatrixZ.m[10] = 1;

	Matrix4x4 RotationMatrixYX = MatrixGetIdentity ();
	MatrixMultiplyMatrix (RotationMatrixY, RotationMatrixX, &RotationMatrixYX);

	Matrix4x4 RotationMatrixYXZ = MatrixGetIdentity ();
	MatrixMultiplyMatrix (RotationMatrixYX, RotationMatrixZ, &RotationMatrixYXZ);

	MatrixMultiplyMatrix (M, RotationMatrixYXZ, Result);
}

void MatrixRotateQuaternion (Matrix4x4 M, Vector4 Rotation, Matrix4x4* Result)
{
	Vector4 Normalized = { 0,0,0 };
	Vector4Normalize (Rotation, &Normalized);

	Matrix4x4 RotationMatrix;

	RotationMatrix.m[0] = 1.0f - 2.0f * Normalized.y * Normalized.y - 2.0f * Normalized.z * Normalized.z;
	RotationMatrix.m[1] = 2.0f * Normalized.x * Normalized.y - 2.0f * Normalized.z * Normalized.w;
	RotationMatrix.m[2] = 2.0f * Normalized.x * Normalized.z + 2.0f * Normalized.y * Normalized.w;
	RotationMatrix.m[3] = 0;
	
	RotationMatrix.m[4] = 2.0f * Normalized.x * Normalized.y + 2.0f * Normalized.z * Normalized.w;
	RotationMatrix.m[5] = 1.0f - 2.0f * Normalized.x * Normalized.x - 2.0f * Normalized.z * Normalized.z;
	RotationMatrix.m[6] = 2.0f * Normalized.y * Normalized.z - 2.0f * Normalized.x * Normalized.w;
	RotationMatrix.m[7] = 0;
	
	RotationMatrix.m[8] = 2.0f * Normalized.x * Normalized.z - 2.0f * Normalized.y * Normalized.w;
	RotationMatrix.m[9] = 2.0f * Normalized.y * Normalized.z + 2.0f * Normalized.x * Normalized.w;
	RotationMatrix.m[10] = 1.0f - 2.0f * Normalized.x * Normalized.x - 2.0f * Normalized.y * Normalized.y;
	RotationMatrix.m[11] = 0;
	
	RotationMatrix.m[12] = 0;
	RotationMatrix.m[13] = 0;
	RotationMatrix.m[14] = 0;
	RotationMatrix.m[15] = 1;

	MatrixMultiplyMatrix (M, RotationMatrix, Result);
}

void MatrixScale (Matrix4x4 M, Vector3 Scale, Matrix4x4* Result)
{
	Matrix4x4 ScaleMatrix = MatrixGetIdentity ();

	ScaleMatrix.m[0] = Scale.x;
	ScaleMatrix.m[5] = Scale.y;
	ScaleMatrix.m[10] = Scale.z;

	MatrixMultiplyMatrix (M, ScaleMatrix, Result);
}