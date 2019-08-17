#pragma once
#include "Maths.h"

#include <stdint.h>

typedef enum _IndicesType
{
	AGAINST_ASSET_INDEX_TYPE_32,
	AGAINST_ASSET_INDEX_TYPE_16,
	AGAINST_ASSET_INDEX_TYPE_8
} IndicesType;

typedef struct _Mesh
{
	Matrix4x4 TransformationMatrix;

	uint32_t VertexCount;
	uint32_t IndexCount;

	float* VertexPositions;
	float* VertexNormals;
	float* VertexUVs;
	float* VertexColors;

	union
	{
		uint32_t* Indices32;
		uint16_t* Indices16;
		uint8_t* Indices8;
	};

	IndicesType IndicesType;

	const char* Name;
	uint32_t ID;
} Mesh;