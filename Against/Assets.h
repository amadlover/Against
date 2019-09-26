#pragma once
#include "Maths.hpp"

#include <stdint.h>

typedef struct _Texture
{
	unsigned int Width;
	unsigned int Height;
	unsigned int BPP;

	unsigned char* Pixels;
} Texture;

typedef struct _Vertex
{
	Vector3 Position;
	Vector2 UV;
	Vector3 Normal;
} Vertex;

typedef struct _Material
{
	char Name[256];
	Texture BaseColorTexture;
} Material;

typedef struct _Primitive
{
	float* Positions;
	float* UVs;
	float* Normals;

	uint32_t PositionSize;
	uint32_t UVSize;
	uint32_t NormalsSize;
	uint32_t IndexSize;

	uint32_t IndexCount;
	uint32_t* Indices;

	Material* Material;
} Primitive;

typedef struct _Mesh
{
	float TransformationMatrix[16];

	float Translation[3];
	float Rotation[4];
	float Scale[3];

	uint32_t IndexCount;

	float* Positions;
	float* UVs;
	float* Normals;

	uint32_t* Indices;

	char Name[256];
	uint32_t ID;

	Material* Materials;

	uint32_t PositionsSize;
	uint32_t NormalsSize;
	uint32_t UVsSize;
	uint32_t IndicesSize;

	Primitive* Primitives;
	uint32_t PrimitiveCount;
} Mesh;