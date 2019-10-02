#include "Maths.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void CreateOrthographicMatrixGLM (float Near, float Far, float Left, float Right, float Bottom, float Top, float* Result)
{
	glm::mat4 Projection = glm::orthoRH (Left, Right, Bottom, Top, Near, Far);
	
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

	memcpy (Result, glm::value_ptr (TranslateMat * ProjectionMat), sizeof (float) * 16);
}