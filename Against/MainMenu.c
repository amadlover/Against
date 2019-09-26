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

Mesh* Meshes;
uint32_t MeshCount;

VkBuffer MainMenuHostVBIB;

VkDeviceMemory MainMenuHostVBIBMemory;

int ImportMainMenuAssets ()
{
	OutputDebugString (L"ImportMainMenuAssets\n");

	TCHAR UIElementPath[MAX_PATH];
	GetApplicationFolder (UIElementPath);
	StringCchCat (UIElementPath, MAX_PATH, L"\\UIElements\\MainMenu\\MainMenu.gltf");

	char UIElementFile[MAX_PATH];
	wcstombs_s (NULL, UIElementFile, MAX_PATH, UIElementPath, MAX_PATH);

	int Result = ImportMainMenuGLTF (UIElementFile, &Meshes, &MeshCount);

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
			VBIBCreateInfo.size += Meshes[m].Primitives[p].PositionSize + Meshes[m].Primitives[p].UVSize + Meshes[m].Primitives[p].IndexSize;
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

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, CurrentMemoryOffset, Meshes[m].Primitives[p].UVSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].Primitives[p].UVs, Meshes[m].Primitives[p].UVSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			CurrentMemoryOffset += Meshes[m].Primitives[p].UVSize;

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
	OutputDebugString (L"CreateMainMenuTextureImages");

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
			if (Meshes[m].Positions)
			{
				free (Meshes[m].Positions);
			}

			if (Meshes[m].UVs)
			{
				free (Meshes[m].UVs);
			}

			if (Meshes[m].Indices)
			{
				free (Meshes[m].Indices);
			}

			if (Meshes[m].Primitives)
			{
				for (uint32_t p = 0; p < Meshes[m].PrimitiveCount; p++)
				{
					if (Meshes[m].Primitives[p].Material)
					{
						free (Meshes[m].Primitives[p].Material);
					}

					if (Meshes[m].Primitives[p].Positions)
					{
						free (Meshes[m].Primitives[p].Positions);
					}

					if (Meshes[m].Primitives[p].UVs)
					{
						free (Meshes[m].Primitives[p].UVs);
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
}