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

VkBuffer MainMenuHostVB;
VkBuffer MainMenuHostIB;

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

/*int CreateMainMenuHostVBIB ()
{
	OutputDebugString (L"CreateMainMenuHostVBIB\n");

	VkBufferCreateInfo VBCreateInfo;
	memset (&VBCreateInfo, 0, sizeof (VkBufferCreateInfo));

	VBCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	VBCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VBCreateInfo.queueFamilyIndexCount = 1;
	VBCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	VBCreateInfo.size = BGMesh->PositionsSize + BGMesh->UVsSize + NewButtonMesh->PositionsSize + NewButtonMesh->UVsSize + QuitButtonMesh->PositionsSize + QuitButtonMesh->UVsSize;

	if (vkCreateBuffer (GraphicsDevice, &VBCreateInfo, NULL, &MainMenuHostVB) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkBufferCreateInfo IBCreateInfo;
	memset (&IBCreateInfo, 0, sizeof (VkBufferCreateInfo));

	IBCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	IBCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	IBCreateInfo.queueFamilyIndexCount = 1;
	IBCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	IBCreateInfo.size = BGMesh->IndicesSize + NewButtonMesh->IndicesSize + QuitButtonMesh->IndicesSize;

	if (vkCreateBuffer (GraphicsDevice, &IBCreateInfo, NULL, &MainMenuHostIB) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements VBMemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, MainMenuHostVB, &VBMemoryRequirements);

	VkMemoryRequirements IBMemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, MainMenuHostIB, &IBMemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = VBMemoryRequirements.size + IBMemoryRequirements.size;
	
	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (VBMemoryRequirements.memoryTypeBits & (1 << i) && IBMemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &MainMenuHostVBIBMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, MainMenuHostVB, MainMenuHostVBIBMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, MainMenuHostIB, MainMenuHostVBIBMemory, VBMemoryRequirements.alignment) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	void* Data;

	if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, 0, BGMesh->PositionsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy (Data, BGMesh->PositionsSize, BGMesh->Positions, BGMesh->PositionsSize);
	vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

	if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, BGMesh->PositionsSize, BGMesh->UVsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy (Data, BGMesh->UVsSize, BGMesh->UVs, BGMesh->UVsSize);
	vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

	if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, BGMesh->PositionsSize + BGMesh->UVsSize, NewButtonMesh->PositionsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy (Data, NewButtonMesh->PositionsSize, NewButtonMesh->Positions, NewButtonMesh->PositionsSize);
	vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

	if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, BGMesh->PositionsSize + BGMesh->UVsSize + NewButtonMesh->PositionsSize, NewButtonMesh->UVsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy (Data, NewButtonMesh->UVsSize, NewButtonMesh->UVs, NewButtonMesh->UVsSize);
	vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

	if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, BGMesh->PositionsSize + BGMesh->UVsSize + NewButtonMesh->PositionsSize + NewButtonMesh->UVsSize, QuitButtonMesh->PositionsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy (Data, QuitButtonMesh->PositionsSize, QuitButtonMesh->Positions, QuitButtonMesh->PositionsSize);
	vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

	if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, BGMesh->PositionsSize + BGMesh->UVsSize + NewButtonMesh->PositionsSize + NewButtonMesh->UVsSize + QuitButtonMesh->PositionsSize, QuitButtonMesh->UVsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy (Data, QuitButtonMesh->UVsSize, QuitButtonMesh->UVs, QuitButtonMesh->UVsSize);
	vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

	return 0;
}*/

int CreateMainMenuHostVBIB ()
{
	OutputDebugString (L"CreateMainMenuHostVBIB\n");

	VkBufferCreateInfo VBCreateInfo;
	memset (&VBCreateInfo, 0, sizeof (VkBufferCreateInfo));

	VBCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	VBCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VBCreateInfo.queueFamilyIndexCount = 1;
	VBCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;

	for (uint32_t m = 0; m < MeshCount; m++)
	{
		for (uint32_t p = 0; p < Meshes[m].PrimitiveCount; p++)
		{
			VBCreateInfo.size += Meshes[m].Primitives[p].PositionsSize + Meshes[m].Primitives[p].UVsSize;
		}
	}

	if (vkCreateBuffer (GraphicsDevice, &VBCreateInfo, NULL, &MainMenuHostVB) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkBufferCreateInfo IBCreateInfo;
	memset (&IBCreateInfo, 0, sizeof (VkBufferCreateInfo));

	IBCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	IBCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	IBCreateInfo.queueFamilyIndexCount = 1;
	IBCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;

	for (uint32_t m = 0; m < MeshCount; m++)
	{
		for (uint32_t p = 0; p < Meshes[m].PrimitiveCount; p++)
		{
			IBCreateInfo.size += Meshes[m].Primitives[p].IndicesSize;
		}
	}

	if (vkCreateBuffer (GraphicsDevice, &IBCreateInfo, NULL, &MainMenuHostIB) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements VBMemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, MainMenuHostVB, &VBMemoryRequirements);

	VkMemoryRequirements IBMemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, MainMenuHostIB, &IBMemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = VBMemoryRequirements.size + IBMemoryRequirements.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (VBMemoryRequirements.memoryTypeBits & (1 << i) && IBMemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &MainMenuHostVBIBMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, MainMenuHostVB, MainMenuHostVBIBMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	VkDeviceSize IndexBufferOffset = (VkDeviceSize)ceil ((double)VBMemoryRequirements.size / (double)VBMemoryRequirements.alignment) * VBMemoryRequirements.size;

	if (vkBindBufferMemory (GraphicsDevice, MainMenuHostIB, MainMenuHostVBIBMemory, IndexBufferOffset) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	void* Data;

	for (uint32_t m = 0; m < MeshCount; m++)
	{
		if (m == 0)
		{
			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, 0, Meshes[m].PositionsSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].Positions, Meshes[m].PositionsSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, Meshes[m].PositionsSize, Meshes[m].UVsSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].UVs, Meshes[m].UVsSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, 0, Meshes[m].IndicesSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].Indices, Meshes[m].IndicesSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);
		}
		else if (m > 0)
		{
			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, Meshes[m - 1].PositionsSize + Meshes[m - 1].UVsSize, Meshes[m].PositionsSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].Positions, Meshes[m].PositionsSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, Meshes[m - 1].PositionsSize + Meshes[m - 1].UVsSize + Meshes[m].PositionsSize, Meshes[m].UVsSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].UVs, Meshes[m].UVsSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);

			if (vkMapMemory (GraphicsDevice, MainMenuHostVBIBMemory, Meshes[m - 1].IndicesSize, Meshes[m].IndicesSize, 0, &Data) != VK_SUCCESS)
			{
				return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
			}

			memcpy (Data, Meshes[m].Indices, Meshes[m].IndicesSize);
			vkUnmapMemory (GraphicsDevice, MainMenuHostVBIBMemory);
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

	if (MainMenuHostVB != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, MainMenuHostVB, NULL);
	}

	if (MainMenuHostIB != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, MainMenuHostIB, NULL);
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

					if (Meshes[m].Primitives[p].Normals)
					{
						free (Meshes[m].Primitives[p].Normals);
					}
				}

				free (Meshes[m].Primitives);
			}
		}

		free (Meshes);
	}
}