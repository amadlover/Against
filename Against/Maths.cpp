#include "Maths.hpp"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

void CreateOrthographicMatrixGLM (float Near, float Far, float Left, float Right, float Bottom, float Top, float* Result)
{
	glm::mat4 Projection = glm::ortho (Left, Right, Bottom, Top, Near, Far);

	memcpy (Result, glm::value_ptr (Projection), sizeof (float) * 16);
}

void CreatePerspectiveProjectionMatrixGLM (float FOV, float AspectRatio, float Near, float Far, float* Result)
{
	glm::mat4 Projection = glm::perspective (FOV, AspectRatio, Near, Far);

	memcpy (Result, glm::value_ptr (Projection), sizeof (float) * 16);
}

void CreateLookatMatrixGLM (float* Eye, float* Center, float* Up, float* Result)
{
	glm::mat4 Transfomation = glm::lookAt (glm::make_vec3 (Eye), glm::make_vec3 (Center), glm::make_vec3 (Up));

	memcpy (Result, glm::value_ptr (Transfomation), sizeof (float) * 16);
}

void CreateModelViewProjectinMatrix (float* Projection, float* View, float* Model, float* Result)
{
	glm::mat4 ProjectionMat = glm::make_mat4 (Projection);
	glm::mat4 ViewMat = glm::make_mat4 (View);
	glm::mat4 ModelMat = glm::make_mat4 (Model);

	memcpy (Result, glm::value_ptr (ProjectionMat * ViewMat * ModelMat), sizeof (float) * 16);
}

void CreateTransformationMatrixGLM (float* Position, float* Rotation, float* Scale, float* Result)
{
	glm::mat4 TranslationMat = glm::translate (glm::mat4 (1.0), glm::make_vec3 (Position));

	glm::quat Quat = glm::make_quat (Rotation);
	glm::mat4 RotationMat = Quat.operator glm::mat<4, 4, float, glm::packed_highp> ();

	glm::mat4 ScaleMat = glm::scale (glm::mat4 (1.0), glm::make_vec3 (Scale));

	memcpy (Result, glm::value_ptr (TranslationMat * RotationMat * ScaleMat), sizeof (float) * 16);
}

void MultiplyMatricesGLM (float* Mat1, float* Mat2, float* Result)
{
	glm::mat4 M1 = glm::make_mat4 (Mat1);
	glm::mat4 M2 = glm::make_mat4 (Mat2);

	memcpy (Result, glm::value_ptr (M1 * M2), sizeof (float) * 16);
}

/*void CreateOrthographicMatrix (float Near, float Far, float Left, float Right, float Bottom, float Top, float* Result)
{
	memset (Result, 0, sizeof (float) * 16);

	Result[0] = 2.f / (Right - Left);

	Result[5] = 2.f / (Bottom - Top);

	Result[10] = 1.f / (Near - Far);

	Result[12] = -(Right + Left) / (Right - Left);
	Result[13] = -(Bottom + Top) / (Bottom - Top);
	Result[14] = Near / (Near - Far);
	Result[15] = 1.f;
}

void CreatePerspectiveMatrix (float AspectRatio, float FOV, float Near, float Far, float* Result)
{
	memset (Result, 0, sizeof (float) * 16);

	float F = 1.f / tan ((0.5f * FOV) * 0.01745329251994329576923690768489f);

	Result[0] = F / AspectRatio;

	Result[5] = -F;

	Result[10] = Far / (Near - Far);
	Result[11] = -1.f;

	Result[14] = (Near - Far) / (Near - Far);
}*/