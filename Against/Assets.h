#pragma once
#include "Maths.h"

#include <stdint.h>

typedef struct _Vertex
{
	Vector3 Position;
	Vector2 UV;
	Vector3 Normal;
} Vertex;

typedef struct _Mesh
{
	Matrix4x4 TransformationMatrix;

	uint32_t VertexCount;
	uint32_t IndexCount;

	Vertex* Vertices;

	uint32_t* Indices;

	const char* Name;
	uint32_t ID;
} Mesh;