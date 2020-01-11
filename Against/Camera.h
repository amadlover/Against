#pragma once

#include "Maths.hpp"

typedef struct _Camera
{
	float TransformationMatrix[16];

	float Position[3];
	float QuatRotation[4];
	float EulerRotation[4];

	float FOV;
} Camera;