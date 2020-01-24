#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct
{
	char Name[256];

	VkDeviceSize Width;
	VkDeviceSize Height;
	VkDeviceSize BPP;

	uint8_t* Pixels;

	VkDeviceSize Size;
	VkDeviceSize Offset;

	VkImage* image;
	uint32_t LayerIndex;
	VkImageView* ImageView;
} image;

typedef struct
{
	char Name[256];

	image* image;
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

int ImportAssets (const char* FilePath, Asset** Assets, uint32_t* AssetCount, image* Images);
void DestroyAssets (Asset* Assets, uint32_t AssetCount);

int import_images (const char* FilePath, image** Images, uint32_t* ImageCount);