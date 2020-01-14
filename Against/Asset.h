#pragma once
#include "Image.h"

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct
{
	VkMemoryRequirements MemoryRequirements;
} VkImageHandles;

typedef struct
{
	char Name[256];
	unsigned int Width;
	unsigned int Height;
	unsigned int BPP;

	unsigned char* Pixels;

	uint32_t Size;

	VkImageHandles VkHandles;
} Image_Orig;

typedef struct
{
	int MinFilter;
	int MagFilter;
	int Wrap_S;
	int Wrap_T;
} Sampler_Orig;

typedef struct _Texture
{
	char Name[256];
	Image_Orig* Image;
	Sampler_Orig* Sampler_Orig;
} Texture_Orig;

typedef struct
{
	char Name[256];
	Texture_Orig* BaseColorTexture;
} Material_Orig;

typedef struct
{
	//TODO: Pointers or values;
	VkDeviceSize Offset;
	VkImage* Image;
	VkImageView* ImageViews;
	VkSampler Sampler_Orig;
	VkDescriptorSet* DescriptorSet;
	VkMemoryRequirements MemoryRequirements;
} VkPrimitiveHandles;

typedef struct
{
	float* Positions;
	float* UV0s;
	float* Normals;

	uint32_t PositionsSize;
	uint32_t UV0Size;
	uint32_t NormalsSize;
	uint32_t IndicesSize;

	uint32_t IndexCount;
	uint32_t* Indices;

	Material_Orig* Material;

	VkPrimitiveHandles VkHandles;
} Primitive_Orig;

typedef struct
{
	char Name[256];
	uint32_t ID;

	Primitive_Orig* Primitives;
	uint32_t PrimitiveCount;
} Mesh_Orig;

typedef struct _Node
{
	float TransformationMatrix[16];

	float Translation[3];
	float Rotation[4];
	float Scale[3];

	char Name[256];

	Mesh_Orig* Mesh_Orig;
} Node_Orig;

typedef struct
{
	char Name[256];

	Image* Image;
} Texture;

typedef struct
{
	char Name[256];

	Texture BaseColorTexture;
} Material;

typedef struct
{
	float* Positions;
	float* UV0s;
	float* Normals;

	VkDeviceSize PositionsSize;
	VkDeviceSize UV0sSize;
	VkDeviceSize NormalsSize;
	VkDeviceSize IndicesSize;

	uint32_t IndexCount;
	uint32_t* Indices;

	VkDeviceSize PositionsOffset;
	VkDeviceSize UV0sOffset;
	VkDeviceSize NormalsOffset;
	VkDeviceSize IndicesOffset;

	Material Material;
} GraphicsPrimitive;

typedef struct
{
	float* Positions;

	VkDeviceSize PositionsSize;
	VkDeviceSize IndicesSize;

	uint32_t IndexCount;
	uint32_t* Indices;
} PhysicsPrimitive;

typedef struct
{
	GraphicsPrimitive* GraphicsPrimPtr;
	PhysicsPrimitive* PhysicsPrimPtr;
} PrimitivePtr;

typedef struct
{
	char Name[256];
	uint32_t ID;

	PhysicsPrimitive* PhysicsPrimitives;
	GraphicsPrimitive* GraphicsPrimitives;

	uint32_t PhysicsPrimitiveCount;
	uint32_t GraphicsPrimitiveCount;
} Asset;

int ImportAssets (const char* FilePath, Asset** Assets, uint32_t* AssetCount, Image* Images);