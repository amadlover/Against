#include "Maths.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void CreateOrthographicMatrixGLM (float Near, float Far, float Left, float Right, float Bottom, float Top, float* Result)
{
	glm::mat4 Projection = glm::ortho (Left, Right, Bottom, Top, Near, Far);
	
	memcpy (Result, glm::value_ptr (Projection), sizeof (float) * 16);
}

void CreateTransformationMatrix (float* Position, float* Rotation, float* Scale, float* Result)
{
	glm::mat4 TranslateMat = glm::translate (glm::mat4 (1.0), glm::vec3 (Position[0], Position[1], Position[2]));

	memcpy (Result, glm::value_ptr (TranslateMat), sizeof (float) * 16);
}

void CreateViewProjectionMatrix (float Near, float Far, float Left, float Right, float Bottom, float Top, float* Position, float* Rotation, float* Scale, float* Result)
{
	glm::mat4 ProjectionMat = glm::orthoRH (Left, Right, Bottom, Top, Near, Far);
	glm::mat4 TranslateMat = glm::translate (glm::mat4 (1.0), glm::vec3 (Position[0], Position[1], Position[2]));
	glm::mat4 LookAt = glm::lookAtRH (glm::vec3 (0, 0, 5), glm::vec3 (0, 0, 0), glm::vec3 (0, 1, 0));

	memcpy (Result, glm::value_ptr (ProjectionMat * LookAt), sizeof (float) * 16);
}

void MultiplyMatrices (float* Mat1, float* Mat2, float* Result)
{
	glm::mat4 M1 = glm::make_mat4 (Mat1);
	glm::mat4 M2 = glm::make_mat4 (Mat2);

	memcpy (Result, glm::value_ptr (M1 * M2), sizeof (float) * 16);
}