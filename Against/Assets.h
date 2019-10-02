#pragma once
#include <stdint.h>

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

	uint32_t PixelSize;

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

	VkPrimitiveHandles VkHandles;
} Primitive;

typedef struct _Mesh
{
	char Name[256];
	uint32_t ID;

	Primitive* Primitives;
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