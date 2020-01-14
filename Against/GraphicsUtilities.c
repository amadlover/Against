#include "GraphicsUtilities.h"
#include "Error.h"
#include "Utility.h"

#include <stdio.h>
#include <stdlib.h>

int CreateBufferAndBufferMemory (VkDevice GraphicsDevice, 
								VkDeviceSize Size, 
								VkBufferUsageFlags Usage, 
								VkSharingMode SharingMode, 
								uint32_t GraphicsQueueFamilyIndex, 
								VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, 
								VkMemoryPropertyFlags RequiredTypes, 
								VkBuffer* OutBuffer, 
								VkDeviceMemory* OutBufferMemory)
{
	VkBufferCreateInfo BufferCreateInfo = { 0 };

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

	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;
	GetMemoryTypeIndex (MemoryRequirements, PhysicalDeviceMemoryProperties, RequiredTypes, &MemoryAllocateInfo.memoryTypeIndex);

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, OutBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, *OutBuffer, *OutBufferMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	return 0;
}

int CopyDataToBuffer (VkDevice GraphicsDevice, 
						VkDeviceMemory Memory, 
						VkDeviceSize Offset, 
						VkDeviceSize Size, 
						void* DataSource)
{
	void* Data;
	if (vkMapMemory (GraphicsDevice, Memory, Offset, Size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, DataSource, (size_t)Size);
	vkUnmapMemory (GraphicsDevice, Memory);

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

int CreateImageAndImageMemory (VkDevice GraphicsDevice, VkExtent3D Extent, uint32_t ArrayLayers)
{
	return 0;
}

int ChangeImageLayout ()
{
	return 0;
}

int CopyBufferToBuffer (VkDevice GraphicsDevice, 
						VkCommandPool CommandPool, 
						VkQueue GraphicsQueue, 
						VkBuffer SrcBuffer, 
						VkBuffer DstBuffer,		
						VkDeviceSize Size)
{
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo = { 0 };

	CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	CommandBufferAllocateInfo.commandPool = CommandPool;
	CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	CommandBufferAllocateInfo.commandBufferCount = 1;

	VkCommandBuffer CommandBuffer;

	if (vkAllocateCommandBuffers (GraphicsDevice, &CommandBufferAllocateInfo, &CommandBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	VkCommandBufferBeginInfo CommandBufferBeginInfo = { 0 };
	CommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer (CommandBuffer, &CommandBufferBeginInfo) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	VkBufferCopy BufferCopy = { 0 };
	BufferCopy.size = Size;

	vkCmdCopyBuffer (CommandBuffer, SrcBuffer, DstBuffer, 1, &BufferCopy);

	vkEndCommandBuffer (CommandBuffer);

	SubmitOneTimeCmd (GraphicsQueue, CommandBuffer);

	vkFreeCommandBuffers (GraphicsDevice, CommandPool, 1, &CommandBuffer);

	return 0;
}

int CopyBufferToImage ()
{
	return 0;
}

int CreateShader (char* FullFilePath, 
					VkDevice GraphicsDevice, 
					VkShaderStageFlagBits ShaderStage, 
					VkShaderModule* ShaderModule, 
					VkPipelineShaderStageCreateInfo* ShaderStageCreateInfos)
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

int GetMemoryTypeIndex (VkMemoryRequirements MemoryRequirements, 
						VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, 
						VkMemoryPropertyFlags RequiredMemoryTypes, 
						uint32_t* MemoryTypeIndex)
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

int SubmitOneTimeCmd (VkQueue GraphicsQueue, VkCommandBuffer CommandBuffer)
{
	VkSubmitInfo SubmitInfo = { 0 };

	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffer;

	if (vkQueueSubmit (GraphicsQueue, 1, &SubmitInfo, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	vkQueueWaitIdle (GraphicsQueue);

	return 0;
}

void DestroyBufferAndBufferMemory (VkDevice GraphicsDevice, VkBuffer Buffer, VkDeviceMemory BufferMemory)
{
	if (Buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (GraphicsDevice, Buffer, NULL);
	}

	if (BufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (GraphicsDevice, BufferMemory, NULL);
	}
}