#pragma once

#include "Maths.h"

typedef struct _MatricesUBO
{
	Matrix4x4 ProjectionMatrix;
	Matrix4x4 ViewMatrix;
	Matrix4x4 ModelMatrix;
} MatricesUBO;
