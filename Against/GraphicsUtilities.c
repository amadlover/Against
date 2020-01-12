#include "GraphicsUtilities.h"
#include "Error.h"
#include "Graphics.h"
#include "Utility.h"

#include <stdio.h>
#include <stdlib.h>

int CreateBufferAndBufferMemory (VkDevice GraphicsDevice, VkDeviceSize Size, VkBufferUsageFlags Usage, VkSharingMode SharingMode, uint32_t GraphicsQueueFamilyIndex, VkMemoryPropertyFlags RequiredTypes, VkBuffer* OutBuffer, VkDeviceMemory* OutBufferMemory)
{
	VkBufferCreateInfo BufferCreateInfo;
	memset (&BufferCreateInfo, 0, sizeof (VkBufferCreateInfo));

	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.size = Size;
	BufferCreateInfo.queueFamilyIndexCount = 1;
	BufferCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;
	BufferCreateInfo.usage = Usage;

	if (vkCreateBuffer (GraphicsDevice, &BufferCreateInfo, NULL, OutBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}
	
	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, *OutBuffer, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
	GetMemoryTypeIndex (MemoryRequirements, PhysicalDeviceMemoryProperties, RequiredTypes, &MemoryAllocateInfo.memoryTypeIndex);

	return 0;
}

errno_t ReadShaderFile (char* FullFilePath, char** FileContents)
{
	FILE* VertFile = NULL;
	errno_t Err = fopen_s (&VertFile, FullFilePath, "rb");

	if (Err != 0)
	{
		return Err;
	}

	fseek (VertFile, 0, SEEK_END);

	uint32_t FileSize = (uint32_t)ftell (VertFile) / sizeof (uint32_t);
	rewind (VertFile);

	*FileContents = (char*)MyMalloc (sizeof (uint32_t) * FileSize);
	fread (*FileContents, sizeof (uint32_t), FileSize, VertFile);
	fclose (VertFile);

	return 0;
}

int CreateShader (char* FullFilePath, VkShaderStageFlagBits ShaderStage, VkShaderModule* ShaderModule, VkPipelineShaderStageCreateInfo* ShaderStageCreateInfos)
{
	char* FileContents = NULL;
	
	FILE* VertFile = NULL;
	errno_t Err = fopen_s (&VertFile, FullFilePath, "rb");

	if (Err != 0)
	{
		return Err;
	}

	fseek (VertFile, 0, SEEK_END);

	uint32_t FileSize = (uint32_t)ftell (VertFile);
	rewind (VertFile);

	FileContents = (char*)MyMalloc (FileSize);
	fread (FileContents, sizeof (uint32_t), FileSize, VertFile);
	fclose (VertFile);

	VkShaderModuleCreateInfo ShaderModuleCreateInfo;
	memset (&ShaderModuleCreateInfo, 0, sizeof (VkShaderModuleCreateInfo));

	ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderModuleCreateInfo.pCode = (uint32_t*)FileContents;
	ShaderModuleCreateInfo.codeSize = FileSize;

	if (vkCreateShaderModule (GraphicsDevice, &ShaderModuleCreateInfo, NULL, ShaderModule) != VK_SUCCESS)
	{
		MyFree (FileContents);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	MyFree (FileContents);

	VkPipelineShaderStageCreateInfo ShaderStageCreateInfo;
	memset (&ShaderStageCreateInfo, 0, sizeof (VkPipelineShaderStageCreateInfo));

	ShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	ShaderStageCreateInfo.stage = ShaderStage;
	ShaderStageCreateInfo.module = *ShaderModule;
	ShaderStageCreateInfo.pName = "main";

	if (ShaderStage == VK_SHADER_STAGE_VERTEX_BIT)
	{
		*ShaderStageCreateInfos = ShaderStageCreateInfo;
	}
	else if (ShaderStage == VK_SHADER_STAGE_FRAGMENT_BIT)
	{
		*ShaderStageCreateInfos = ShaderStageCreateInfo;
	}

	return 0;
}

int CreateTextureImageOnDevice (char* FilePath, VkImage* TextureImage, VkDeviceMemory* TextureImageMemory)
{
	return 0;
}

int GetMemoryTypeIndex (VkMemoryRequirements MemoryRequirements, VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, VkMemoryPropertyFlags RequiredMemoryTypes, uint32_t* MemoryTypeIndex)
{
	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << i) && RequiredMemoryTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			*MemoryTypeIndex = i;
			break;
		}
	}

	return 0;
}