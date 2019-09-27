#pragma once
#include "Maths.hpp"

#include <stdint.h>

#include <vulkan/vulkan.h>

typedef struct _Image
{
	char Name[256];
	unsigned int Width;
	unsigned int Height;
	unsigned int BPP;

	unsigned char* Pixels;

	uint32_t PixelSize;
}Image;

typedef struct _Texture
{
	char Name[256];
	Image* Image;
} Texture;

typedef struct _Material
{
	char Name[256];
	Texture* BaseColorTexture;
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

	Material* Material;
} Primitive;

typedef struct _Mesh
{
	float TransformationMatrix[16];

	float Translation[3];
	float Rotation[4];
	float Scale[3];

	char Name[256];
	uint32_t ID;

	Primitive* Primitives;
	uint32_t PrimitiveCount;
} Mesh;