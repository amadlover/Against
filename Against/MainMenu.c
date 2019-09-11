#include "MainMenu.h"

#include "Error.h"
#include "ImportAssets.h"
#include "Utility.h"
#include "Graphics.h"

#include <Windows.h>
#include <strsafe.h>

#include <stdlib.h>

#include <cgltf.h>

#include <vulkan/vulkan.h>

cgltf_data* MainMenuData = NULL;

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

	MainMenuData = ImportGLTF (UIElementFile);

	if (MainMenuData == NULL)
	{
		return AGAINST_ERROR_GLTF_COULD_NOT_IMPORT;
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

	uint64_t VertexMemorySize = 0;

	uint64_t BGVertexBufferSize = 0;
	uint64_t UIVertexBufferSize = 0;
	uint64_t BGIndexBufferSize = 0;
	uint64_t UIIndexBufferSize = 0;

	for (cgltf_size m = 0; m < MainMenuData->meshes_count; m++)
	{
		uint64_t BufferSize = 0;
		cgltf_mesh* Mesh = MainMenuData->meshes + m;
		
		for (cgltf_size p = 0; p < Mesh->primitives_count; p++)
		{
			cgltf_primitive* Primitive = Mesh->primitives + p;

			for (cgltf_size a = 0; a < Primitive->attributes_count; a++)
			{
				cgltf_attribute* Attribute = Primitive->attributes + a;

				if (strcmp (Attribute->name, "POSITION") == 0)
				{
					BufferSize += Attribute->data->count * sizeof (float) * 3;

					if (strcmp (Mesh->name, "BackgroundPlane") == 0)
					{
						BGVertexBufferSize += Attribute->data->count * sizeof (float) * 3;
					}
					else if (strcmp (Mesh->name, "UIElementPlane") == 0)
					{
						UIVertexBufferSize += Attribute->data->count * sizeof (float) * 3;
					}
				}
				else if (strcmp (Attribute->name, "TEXCOORD_0") == 0)
				{
					BufferSize += Attribute->data->count * sizeof (float) * 2;
					BGVertexBufferSize += Attribute->data->count * sizeof (float) * 3;

					if (strcmp (Mesh->name, "BackgroundPlane") == 0)
					{
						BGVertexBufferSize += Attribute->data->count * sizeof (float) * 2;
					}
					else if (strcmp (Mesh->name, "UIElementPlane") == 0)
					{
						UIVertexBufferSize += Attribute->data->count * sizeof (float) * 2;
					}
				}
			}

			CreateInfo.size = BufferSize;
			VertexMemorySize += BufferSize;

			if (strcmp (Mesh->name, "BackgroundPlane") == 0)
			{
				if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &BGVertexBuffer) != VK_SUCCESS)
				{
					return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
				}

				CreateInfo.size = Primitive->indices->count * sizeof (uint32_t);
				VertexMemorySize += Primitive->indices->count * sizeof (uint32_t);
				BGIndexBufferSize = Primitive->indices->count * sizeof (uint32_t);

				if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &BGIndexBuffer) != VK_SUCCESS)
				{
					return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
				}
			}
			else if (strcmp (Mesh->name, "UIElementPlane") == 0)
			{
				if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &UIVertexBuffer) != VK_SUCCESS)
				{
					return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
				}

				CreateInfo.size = Primitive->indices->count * sizeof (uint32_t);
				VertexMemorySize += Primitive->indices->count * sizeof (uint32_t);
				UIIndexBufferSize = Primitive->indices->count * sizeof (uint32_t);

				if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, &UIIndexBuffer) != VK_SUCCESS)
				{
					return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
				}
			}
		}
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

	cgltf_free (MainMenuData);

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
}