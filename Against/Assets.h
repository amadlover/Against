#pragma once
#include "Maths.h"

#include <stdint.h>

typedef struct _Mesh
{
	Matrix4x4 TransformationMatrix;

	uint32_t VertexCount;
	uint32_t IndexCount;

	float* VertexPositions;
	float* VertexNormals;
	float* VertexUVs;
	float* VertexColors;

	uint32_t* Indices;

	const char* Name;
	uint32_t ID;
} Mesh;