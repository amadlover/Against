#include "graphics_utils.h"
#include "error.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

int CreateBuffer (VkDevice GraphicsDevice, 
								VkDeviceSize Size, 
								VkBufferUsageFlags Usage, 
								VkSharingMode SharingMode, 
								uint32_t GraphicsQueueFamilyIndex, 
								VkBuffer* OutBuffer)
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

	return 0;
}

int AllocateBindBufferMemory (VkDevice GraphicsDevice, 
							VkBuffer* Buffers, 
							uint32_t BufferCount,
							VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, 
							VkMemoryPropertyFlags RequiredTypes, 
							VkDeviceMemory* OutBufferMemory)
{
	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };
	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkDeviceSize* Offsets = (VkDeviceSize*)MyCalloc (BufferCount, sizeof (VkDeviceSize));

	for (uint32_t b = 0; b < BufferCount; b++)
	{
		Offsets[b] = MemoryAllocateInfo.allocationSize;
		
		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements (GraphicsDevice, Buffers[b], &MemoryRequirements);

		MemoryAllocateInfo.allocationSize += MemoryRequirements.size;
		GetMemoryTypeIndex (MemoryRequirements, PhysicalDeviceMemoryProperties, RequiredTypes, &MemoryAllocateInfo.memoryTypeIndex);
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, OutBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	for (uint32_t b = 0; b < BufferCount; b++)
	{
		if (vkBindBufferMemory (GraphicsDevice, Buffers[b], *OutBufferMemory, Offsets[b]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		}
	}

	MyFree (Offsets);

	return 0;
}

int MapDataToBuffer (VkDevice GraphicsDevice, 
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

int CreateImage (
	VkDevice GraphicsDevice, 
	uint32_t GraphicsQueueFamilyIndex, 
	VkExtent3D Extent, 
	uint32_t ArrayLayers, 
	VkFormat Format, 
	VkImageLayout InitialLayout, 
	VkSharingMode SharingMode, 
	VkImage* OutImage
)
{
	VkImageCreateInfo ImageCreateInfo = { 0 };

	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.arrayLayers = ArrayLayers;
	ImageCreateInfo.extent = Extent;
	ImageCreateInfo.format = Format;
	ImageCreateInfo.mipLevels = 0;
	ImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	ImageCreateInfo.initialLayout = InitialLayout;
	ImageCreateInfo.sharingMode = SharingMode;
	ImageCreateInfo.queueFamilyIndexCount = 1;
	ImageCreateInfo.pQueueFamilyIndices = &GraphicsQueueFamilyIndex;

	if (vkCreateImage (GraphicsDevice, &ImageCreateInfo, NULL, OutImage) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
	}

	return 0;
}

int AllocateBindImageMemory (VkDevice GraphicsDevice, VkImage* Images, uint32_t ImageCount, VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, VkMemoryPropertyFlags RequiredTypes, VkDeviceMemory* OutMemory)
{
	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkDeviceSize* Offsets = (VkDeviceSize*)MyCalloc (ImageCount, sizeof (VkDeviceSize));

	for (uint32_t i = 0; i < ImageCount; i++)
	{
		VkMemoryRequirements MemoryRequirements = { 0 };
		vkGetImageMemoryRequirements (GraphicsDevice, Images[i], &MemoryRequirements);

		MemoryAllocateInfo.allocationSize += MemoryRequirements.size;
		
		GetMemoryTypeIndex (MemoryRequirements, PhysicalDeviceMemoryProperties, RequiredTypes, &MemoryAllocateInfo.memoryTypeIndex);
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, OutMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	for (uint32_t i = 0; i < ImageCount; i++)
	{
		if (vkBindImageMemory (GraphicsDevice, Images[i], *OutMemory, Offsets[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		}
	}

	MyFree (Offsets);

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

void DestroyBufferAndBufferMemory (VkDevice GraphicsDevice, 
									VkBuffer Buffer, 
									VkDeviceMemory BufferMemory)
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