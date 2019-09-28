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

Node* Nodes;
uint32_t NodeCount;

Mesh* Meshes;
uint32_t MeshCount;

Material* Materials;
uint32_t MaterialCount;

Texture* Textures;
uint32_t TextureCount;

Image* Images;
uint32_t ImageCount;

VkBuffer MainMenuHostVBIB;

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

	int Result = ImportMainMenuGLTF (UIElementFile, &Nodes, &NodeCount, &Meshes, &MeshCount, &Materials, &MaterialCount, &Textures, &TextureCount, &Images, &ImageCount);

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

	for (uint32_t m = 0; m < MeshCount; m++)
	{
		for (uint32_t p = 0; p < Meshes[m].PrimitiveCount; p++)
		{
			VBIBCreateInfo.size += Meshes[m].Primitives[p].PositionSize + Meshes[m].Primitives[p].UV0Size + Meshes[m].Primitives[p].IndexSize;
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

	for (uint32_t m = 0; m < MeshCount; m++)
	{
		for (uint32_t p = 0; p < Meshes[m].PrimitiveCount; p++)
		{
			void* Data = NULL;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, CurrentMemoryOffset, Meshes[m].Primitives[p].PositionSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].Primitives[p].Positions, Meshes[m].Primitives[p].PositionSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);
			
			CurrentMemoryOffset += Meshes[m].Primitives[p].PositionSize;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, CurrentMemoryOffset, Meshes[m].Primitives[p].UV0Size, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].Primitives[p].UV0s, Meshes[m].Primitives[p].UV0Size);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			CurrentMemoryOffset += Meshes[m].Primitives[p].UV0Size;

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, CurrentMemoryOffset, Meshes[m].Primitives[p].IndexSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].Primitives[p].Indices, Meshes[m].Primitives[p].IndexSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			CurrentMemoryOffset += Meshes[m].Primitives[p].IndexSize;
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

	uint32_t TotalTextureSize = 0;

	for (uint32_t m = 0; m < MeshCount; m++)
	{
		for (uint32_t p = 0; p < Meshes[m].PrimitiveCount; p++)
		{
			/*CreateInfo.extent.width = Meshes[m].Primitives[p].Material.BaseColorTexture.Width;
			CreateInfo.extent.height = Meshes[m].Primitives[p].Material.BaseColorTexture.Height;
			CreateInfo.extent.depth = 1;

			if (vkCreateImage (GraphicsDevice, &CreateInfo, NULL, &Meshes[m].Primitives[p].Material.BaseColorTexture.VulkanHandle) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
			}

			TotalTextureSize += Meshes[m].Primitives[p].Material.BaseColorTexture.Width * Meshes[m].Primitives[p].Material.BaseColorTexture.Height * 4 * sizeof (unsigned char);

			VkMemoryRequirements MemoryRequirements;
			vkGetImageMemoryRequirements (GraphicsDevice, Meshes[m].Primitives[p].Material.BaseColorTexture.VulkanHandle, &MemoryRequirements);*/
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

	if (Meshes)
	{
		for (uint32_t m = 0; m < MeshCount; m++)
		{
			if (Meshes[m].Primitives)
			{
				for (uint32_t p = 0; p < Meshes[m].PrimitiveCount; p++)
				{
					if (Meshes[m].Primitives[p].Positions)
					{
						free (Meshes[m].Primitives[p].Positions);
					}

					if (Meshes[m].Primitives[p].UV0s)
					{
						free (Meshes[m].Primitives[p].UV0s);
					}

					if (Meshes[m].Primitives[p].Indices)
					{
						free (Meshes[m].Primitives[p].Indices);
					}
				}

				free (Meshes[m].Primitives);
			}
		}

		free (Meshes);
	}

	if (Images)
	{
		for (uint32_t i = 0; i < ImageCount; i++)
		{
			if (Images[i].Pixels)
			{
				free (Images[i].Pixels);
			}
		}

		free (Images);
	}

	if (Textures)
	{
		free (Textures);
	}

	if (Materials)
	{
		free (Materials);
	}
}