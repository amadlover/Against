#pragma once
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
} Sampler;

typedef struct _Texture
{
	char Name[256];
	Image_Orig* Image;
	Sampler* Sampler;
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
	VkImageView* ImageView;
	VkSampler Sampler;
	VkDescriptorSet* DescriptorSet;
	VkMemoryRequirements MemoryRequirements;
} VkPrimitiveHandles;

typedef struct
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

	Material_Orig* Material;

	VkPrimitiveHandles VkHandles;
} Primitive_Orig;

typedef struct
{
	char Name[256];
	uint32_t ID;

	Primitive_Orig* Primitives;
	uint32_t PrimitiveCount;
} Mesh;

typedef struct _Node
{
	float TransformationMatrix[16];

	float Translation[3];
	float Rotation[4];
	float Scale[3];

	char Name[256];

	Mesh* Mesh;
} Node;

typedef struct
{
	char Name[256];

	VkDeviceSize Width;
	VkDeviceSize Height;
	VkDeviceSize BPP;

	uint8_t* Pixels;

	VkDeviceSize Size;
} Image;

typedef struct
{
	char Name[256];

	Image Image;
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

	VkDeviceSize PositionSize;
	VkDeviceSize UV0Size;
	VkDeviceSize NormalSize;
	VkDeviceSize IndexSize;

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

	VkDeviceSize PositionSize;
	VkDeviceSize IndexSize;

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

int ImportAssets (const char* FilePath, Asset** Assets, uint32_t* AssetCount);