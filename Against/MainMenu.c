#include "MainMenu.h"

#include "Error.h"
#include "ImportAssets.h"
#include "Utility.h"
#include "Graphics.h"

#include <Windows.h>
#include <strsafe.h>

#include <stdlib.h>
#include <math.h>

#include <vulkan/vulkan.h>

Node* MainMenuNodes;
uint32_t MainMenuNodeCount;

Mesh* MainMenuMeshes;
uint32_t MainMenuMeshCount;

Material* MainMenuMaterials;
uint32_t MainMenuMaterialCount;

Texture* MainMenuTextures;
uint32_t MainMenuTextureCount;

Image* MainMenuImages;
uint32_t MainMenuImageCount;

Sampler* MainMenuSamplers;
uint32_t MainMenuSamplerCount;

VkBuffer MainMenuHostVBIB;

VkImage* MainMenuTImages;
VkImageView* MainMenuTImageViews;

VkDeviceMemory MainMenuHostVBIBMemory;
VkDeviceMemory MainMenuHostTextureMemory;

VkSampler MainMenuSampler;
VkImageView MainMenuImageView;

int ImportMainMenuAssets ()
{
	OutputDebugString (L"ImportMainMenuAssets\n");

	TCHAR UIElementPath[MAX_PATH];
	GetApplicationFolder (UIElementPath);
	StringCchCat (UIElementPath, MAX_PATH, L"\\UIElements\\MainMenu\\MainMenu.gltf");

	char UIElementFile[MAX_PATH];
	wcstombs_s (NULL, UIElementFile, MAX_PATH, UIElementPath, MAX_PATH);

	int Result = ImportMainMenuGLTF (UIElementFile, &MainMenuNodes, &MainMenuNodeCount, &MainMenuMeshes, &MainMenuMeshCount, &MainMenuMaterials, &MainMenuMaterialCount, &MainMenuTextures, &MainMenuTextureCount, &MainMenuImages, &MainMenuImageCount, &MainMenuSamplers, &MainMenuSamplerCount);

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

int CreateMainMenuUniformBuffer ()
{
	OutputDebugString (L"CreateMainScreenUniformBuffer\n");
	
	return 0;
}

int CreateMainMenuHostVBIB ()
{
	OutputDebugString (L"CreateMainMenuHostVBIB\n");

	VkBufferCreateInfo VBIBCreateInfo;
	memset (&VBIBCreateInfo, 0, sizeof (VkBufferCreateInfo));

	VBIBCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	VBIBCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VBIBCreateInfo.queueFamilyIndexCount = 1;
	VBIBCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			VBIBCreateInfo.size += MainMenuMeshes[m].Primitives[p].PositionSize + MainMenuMeshes[m].Primitives[p].UV0Size + MainMenuMeshes[m].Primitives[p].IndexSize;
		}
	}

	if (vkCreateBuffer (GraphicsDevice, &VBIBCreateInfo, NULL, &MainMenuHostVBIB) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements VBIBMemoryRequirements;
	memset (&VBIBMemoryRequirements, 0, sizeof (VkMemoryRequirements));

	vkGetBufferMemoryRequirements (GraphicsDevice, MainMenuHostVBIB, &VBIBMemoryRequirements);

	VkMemoryAllocateInfo VBIBMemoryAllocateInfo;
	memset (&VBIBMemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	VBIBMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VBIBMemoryAllocateInfo.allocationSize = VBIBMemoryRequirements.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (VBIBMemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			VBIBMemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &VBIBMemoryAllocateInfo, NULL, &MainMenuHostVBIBMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, MainMenuHostVBIB, MainMenuHostVBIBMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	VkDeviceSize CurrentMemoryOffset = 0;

	for (uint32_t m = 0; m < MainMenuMeshCount; m++)
	{
		for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
		{
			void* Data = NULL;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, CurrentMemoryOffset, MainMenuMeshes[m].Primitives[p].PositionSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, MainMenuMeshes[m].Primitives[p].Positions, MainMenuMeshes[m].Primitives[p].PositionSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);
			
			CurrentMemoryOffset += MainMenuMeshes[m].Primitives[p].PositionSize;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, CurrentMemoryOffset, MainMenuMeshes[m].Primitives[p].UV0Size, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, MainMenuMeshes[m].Primitives[p].UV0s, MainMenuMeshes[m].Primitives[p].UV0Size);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			CurrentMemoryOffset += MainMenuMeshes[m].Primitives[p].UV0Size;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, CurrentMemoryOffset, MainMenuMeshes[m].Primitives[p].IndexSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, MainMenuMeshes[m].Primitives[p].Indices, MainMenuMeshes[m].Primitives[p].IndexSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			CurrentMemoryOffset += MainMenuMeshes[m].Primitives[p].IndexSize;
		}
	}

	return 0;
}

int CreateMainMenuTextureImages ()
{
	OutputDebugString (L"CreateMainMenuTextureImages\n");

	VkImageCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkImageCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	CreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	CreateInfo.arrayLayers = 1;
	CreateInfo.imageType = VK_IMAGE_TYPE_2D;
	CreateInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	CreateInfo.mipLevels = 1;
	CreateInfo.queueFamilyIndexCount = 1;
	CreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	CreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	CreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;

	MainMenuTImages = (VkImage*)malloc (sizeof (VkImage) * MainMenuTextureCount);

	for (uint32_t t = 0; t < MainMenuTextureCount; t++)
	{
		CreateInfo.extent.width = MainMenuTextures[t].Image->Width;
		CreateInfo.extent.height = MainMenuTextures[t].Image->Height;
		CreateInfo.extent.depth = 1;

		if (vkCreateImage (GraphicsDevice, &CreateInfo, NULL, &MainMenuTImages[t]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
		}
	}

	return 0;
}

int CreateMainMenuShaders ()
{
	OutputDebugString (L"CreateMainMenuShaders\n");

	return 0;
}

int CreateMainMenuFramebuffers ()
{
	OutputDebugString (L"CreateMainMenuFramebuffers\n");

	return 0;
}

int CreateMainMenuRenderPass ()
{
	OutputDebugString (L"CreateMainMenuRenderPass\n");

	return 0;
}

int CreateMainMenuDescriptorSetPool ()
{
	OutputDebugString (L"CreateMainMenuDescriptorSetPool\n");

	return 0;
}

int CreateMainMenuDescriptorSetLayout ()
{
	OutputDebugString (L"CreateMainMenuDescriptorSetLayout\n");

	return 0;
}

int CreateMainMenuDescriptorSet ()
{
	OutputDebugString (L"CreateMainMenuDescriptorSet\n");

	return 0;
}

int CreateMainMenuCommandBuffers ()
{
	OutputDebugString (L"CreateMainMenuCommandBuffers\n");

	return 0;
}

int CreateMainMenuGraphics ()
{
	OutputDebugString (L"SetupMainMenu\n");

	int Result = ImportMainMenuAssets ();
	
	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuUniformBuffer ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuShaders ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuHostVBIB ();

	if (Result != 0)
	{
		return Result;
	}

	Result = CreateMainMenuTextureImages ();

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

int DrawMainMenu ()
{
	return 0;
}

void DestroyMainMenuGraphics ()
{
	OutputDebugString (L"DestroyMainMenu\n");

	if (MainMenuHostVBIB != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, MainMenuHostVBIB, NULL);
	}

	if (MainMenuHostVBIBMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (GraphicsDevice, MainMenuHostVBIBMemory, NULL);
	}

	if (MainMenuTImages)
	{
		for (uint32_t i = 0; i < MainMenuImageCount; i++)
		{
			vkDestroyImage (GraphicsDevice, MainMenuTImages[i], NULL);
		}
	}

	if (MainMenuTImageViews)
	{
		for (uint32_t i = 0; i < MainMenuImageCount; i++)
		{
			vkDestroyImageView (GraphicsDevice, MainMenuTImageViews[i], NULL);
		}
	}

	if (MainMenuNodes)
	{
		free (MainMenuNodes);
	}

	if (MainMenuMeshes)
	{
		for (uint32_t m = 0; m < MainMenuMeshCount; m++)
		{
			if (MainMenuMeshes[m].Primitives)
			{
				for (uint32_t p = 0; p < MainMenuMeshes[m].PrimitiveCount; p++)
				{
					if (MainMenuMeshes[m].Primitives[p].Positions)
					{
						free (MainMenuMeshes[m].Primitives[p].Positions);
					}

					if (MainMenuMeshes[m].Primitives[p].UV0s)
					{
						free (MainMenuMeshes[m].Primitives[p].UV0s);
					}

					if (MainMenuMeshes[m].Primitives[p].Indices)
					{
						free (MainMenuMeshes[m].Primitives[p].Indices);
					}
				}

				free (MainMenuMeshes[m].Primitives);
			}
		}

		free (MainMenuMeshes);
	}

	if (MainMenuImages)
	{
		for (uint32_t i = 0; i < MainMenuImageCount; i++)
		{
			if (MainMenuImages[i].Pixels)
			{
				free (MainMenuImages[i].Pixels);
			}
		}

		free (MainMenuImages);
	}

	if (MainMenuTextures)
	{
		free (MainMenuTextures);
	}

	if (MainMenuMaterials)
	{
		free (MainMenuMaterials);
	}
}