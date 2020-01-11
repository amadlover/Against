#pragma once
#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct _VkImageHandles
{
	VkMemoryRequirements MemoryRequirements;
} VkImageHandles;

typedef struct _Image
{
	char Name[256];
	unsigned int Width;
	unsigned int Height;
	unsigned int BPP;

	unsigned char* Pixels;

	uint32_t Size;

	VkImageHandles VkHandles;
}Image;

typedef struct _Sampler
{
	int MinFilter;
	int MagFilter;
	int Wrap_S;
	int Wrap_T;
} Sampler;

typedef struct _Texture
{
	char Name[256];
	Image* Image;
	Sampler* Sampler;
} Texture;

typedef struct _Material
{
	char Name[256];
	Texture* BaseColorTexture;
} Material;

typedef struct _VkPrimitiveHandles
{
	//TODO: Pointers or values;
	VkBuffer VBIB;
	VkImage* Image;
	VkImageView* ImageView;
	VkSampler Sampler;
	VkDescriptorSet* DescriptorSet;
	VkMemoryRequirements MemoryRequirements;
} VkPrimitiveHandles;

typedef struct _Primitive_Orig
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

	VkPrimitiveHandles VkHandles;
} Primitive_Orig;

typedef struct _Mesh
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

typedef struct _GraphicsPrimitive
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

	VkPrimitiveHandles VkHandles;
} GraphicsPrimitive;

typedef struct _PhysicsPrimitive
{
	float* Positions;

	uint32_t PositionsSize;

	uint32_t IndexCount;
	uint32_t* Indices;

} PhysicsPrimitive;

typedef struct _Asset
{
	char Name[256];
	uint32_t ID;

	PhysicsPrimitive* PhysicsPrimitives;
	GraphicsPrimitive* GraphicsPrimitives;

	uint32_t PhysicsPrimitiveCount;
	uint32_t GraphicsPrimitiveCount;
} Asset;

int ImportAssets (const char* FilePath, Asset** Assets, uint32_t* AssetCount);