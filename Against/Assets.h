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

typedef struct _Material
{
	char Name[256];
	Texture BaseColorTexture;
} Material;

typedef struct _Primitive
{
	float* Positions;
	float* UV0s;
	float* Normals;

	uint32_t PositionSize;
	uint32_t UV0Size;
	uint32_t NormalSize;
	uint32_t IndexSize;

	uint32_t IndexCount;
	uint32_t* Indices;

	Material Material;
} Primitive;

typedef struct _Mesh
{
	float TransformationMatrix[16];

	float Translation[3];
	float Rotation[4];
	float Scale[3];

	uint32_t IndexCount;

	float* Positions;
	float* UV0s;
	float* Normals;

	uint32_t* Indices;

	char Name[256];
	uint32_t ID;

	Material* Materials;

	uint32_t PositionSize;
	uint32_t NormalSize;
	uint32_t UV0Size;
	uint32_t IndexSize;

	Primitive* Primitives;
	uint32_t PrimitiveCount;
} Mesh;