#include "MainMenu.h"

#include "Error.h"
#include "ImportAssets.h"
#include "Utility.h"
#include "Graphics.h"

#include <Windows.h>
#include <strsafe.h>

#include <stdlib.h>

#include <vulkan/vulkan.h>

Mesh Meshes[2];
Mesh *BGMesh;
Mesh *UIElementMesh;
uint32_t MeshCount = 2;

VkBuffer BGVertexBuffer;
VkBuffer UIVertexBuffer;

VkBuffer BGIndexBuffer;
VkBuffer UIIndexBuffer;

VkDeviceMemory BGUIVertexIndexMemory;

int ImportMainMenuAssets ()
{
	OutputDebugString (L"ImportMainMenuAssets\n");
	
	TCHAR UIElementPath[MAX_PATH];
	GetApplicationFolder (UIElementPath);
	StringCchCat (UIElementPath, MAX_PATH, L"\\UIElements\\MainMenu\\MainMenu.gltf");

	char UIElementFile[MAX_PATH];
	wcstombs_s (NULL, UIElementFile, MAX_PATH, UIElementPath, MAX_PATH);

	//Meshes = (Mesh*)malloc (sizeof (Mesh) * MeshCount);

	if (Meshes != NULL)
	{
		int Result = ImportMainMenuGLTF (UIElementFile, Meshes);

		if (Result != 0)
		{
			return Result;
		}

		for (uint32_t m = 0; m < MeshCount; m++)
		{
			if (strcmp (Meshes[m].Name, "BackgroundPlane") == 0)
			{
				BGMesh = &Meshes[m];
			}
			else if (strcmp (Meshes[m].Name, "UIElementPlane") == 0)
			{
				UIElementMesh = &Meshes[m];
			}
		}
	}

	return 0;
}

int CreateMainMenuUniformBuffer ()
{
	OutputDebugString (L"CreateMainScreenUniformBuffer\n");
	
	return 0;
}

int CreateMainMenuHostVBs ()
{
	OutputDebugString (L"CreateMainMenuHostVBs\n");

	VkBufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkBufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	CreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	CreateInfo.queueFamilyIndexCount = 1;
	CreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	CreateInfo.size = (VkDeviceSize)(BGMesh->PositionsSize + BGMesh->UVsSize);

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &BGVertexBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	CreateInfo.size = (VkDeviceSize)(BGMesh->IndicesSize);

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &BGIndexBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	CreateInfo.size = (VkDeviceSize)((UIElementMesh->PositionsSize) + (UIElementMesh->UVsSize));

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &UIVertexBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	CreateInfo.size = (VkDeviceSize)(UIElementMesh->IndicesSize);

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &UIIndexBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements BGVertexBufferMemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, BGVertexBuffer, &BGVertexBufferMemoryRequirements);

	VkMemoryRequirements UIVertexBufferMemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, UIVertexBuffer, &UIVertexBufferMemoryRequirements);

	VkMemoryRequirements BGIndexBufferMemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, BGIndexBuffer, &BGIndexBufferMemoryRequirements);

	VkMemoryRequirements UIIndexBufferMemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, UIIndexBuffer, &UIIndexBufferMemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (MemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = BGVertexBufferMemoryRequirements.size + BGIndexBufferMemoryRequirements.size + UIVertexBufferMemoryRequirements.size + UIIndexBufferMemoryRequirements.size;

	uint32_t RequiredTypes = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (BGVertexBufferMemoryRequirements.memoryTypeBits & (1 << i) && BGIndexBufferMemoryRequirements.memoryTypeBits & (1 << i) && UIVertexBufferMemoryRequirements.memoryTypeBits & (1 << i) && UIIndexBufferMemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, &BGUIVertexIndexMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, BGVertexBuffer, BGUIVertexIndexMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, UIVertexBuffer, BGUIVertexIndexMemory, BGVertexBufferMemoryRequirements.alignment) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, BGIndexBuffer, BGUIVertexIndexMemory, UIVertexBufferMemoryRequirements.alignment) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, UIIndexBuffer, BGUIVertexIndexMemory, BGIndexBufferMemoryRequirements.alignment) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	void* Data = NULL;

	if (vkMapMemory (GraphicsDevice, BGUIVertexIndexMemory, 0, BGMesh->PositionsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy_s (Data, BGMesh->PositionsSize, BGMesh->Positions, BGMesh->PositionsSize);

	vkUnmapMemory (GraphicsDevice, BGUIVertexIndexMemory);

	if (vkMapMemory (GraphicsDevice, BGUIVertexIndexMemory, BGMesh->PositionsSize, BGMesh->UVsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy_s (Data, BGMesh->UVsSize, BGMesh->UVs, BGMesh->UVsSize);

	vkUnmapMemory (GraphicsDevice, BGUIVertexIndexMemory);

	if (vkMapMemory (GraphicsDevice, BGUIVertexIndexMemory, BGMesh->PositionsSize + BGMesh->UVsSize, UIElementMesh->PositionsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy_s (Data, UIElementMesh->PositionsSize, UIElementMesh->Positions, UIElementMesh->PositionsSize);

	vkUnmapMemory (GraphicsDevice, BGUIVertexIndexMemory);

	if (vkMapMemory (GraphicsDevice, BGUIVertexIndexMemory, BGMesh->PositionsSize + BGMesh->UVsSize + UIElementMesh->PositionsSize, UIElementMesh->UVsSize, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_BUFFER_MEMORY;
	}

	memcpy_s (Data, UIElementMesh->UVsSize, UIElementMesh->UVs, UIElementMesh->UVsSize);

	vkUnmapMemory (GraphicsDevice, BGUIVertexIndexMemory);

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

	Result = CreateMainMenuHostVBs ();

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

	if (BGVertexBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, BGVertexBuffer, NULL);
	}

	if (BGIndexBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, BGIndexBuffer, NULL);
	}

	if (UIVertexBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, UIVertexBuffer, NULL);
	}

	if (UIIndexBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, UIIndexBuffer, NULL);
	}

	if (BGUIVertexIndexMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (GraphicsDevice, BGUIVertexIndexMemory, NULL);
	}

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
	}
}