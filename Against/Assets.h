#pragma once
#include "Maths.hpp"

#include <stdint.h>

typedef struct _Vertex
{
	Vector3 Position;
	Vector2 UV;
	Vector3 Normal;
} Vertex;

typedef struct _Material
{
	char* Name;
} Material;

typedef struct _Mesh
{
	Matrix4x4 TransformationMatrix;

	uint32_t VertexCount;
	uint32_t IndexCount;

	Vertex* Vertices;

	float* Positions;
	float* UVs;
	float* Normals;

	uint32_t* Indices;

	char* Name;
	uint32_t ID;

	Material* Materials;
} Mesh;