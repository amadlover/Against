#include "graphics_utils.h"
#include "error.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

int CreateBuffer (VkDevice graphics_device,
	VkDeviceSize Size,
	VkBufferUsageFlags Usage,
	VkSharingMode SharingMode,
	uint32_t graphics_queue_family_index,
	VkBuffer* OutBuffer)
{
	VkBufferCreateInfo BufferCreateInfo = { 0 };

	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.size = Size;
	BufferCreateInfo.queueFamilyIndexCount = 1;
	BufferCreateInfo.pQueueFamilyIndices = &graphics_queue_family_index;
	BufferCreateInfo.usage = Usage;

	if (vkCreateBuffer (graphics_device, &BufferCreateInfo, NULL, OutBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	return 0;
}

int AllocateBindBufferMemory (VkDevice graphics_device,
	VkBuffer* Buffers,
	uint32_t BufferCount,
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties,
	VkMemoryPropertyFlags RequiredTypes,
	VkDeviceMemory* OutBufferMemory)
{
	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };
	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkDeviceSize* Offsets = (VkDeviceSize*)my_calloc (BufferCount, sizeof (VkDeviceSize));

	for (uint32_t b = 0; b < BufferCount; b++)
	{
		Offsets[b] = MemoryAllocateInfo.allocationSize;

		VkMemoryRequirements MemoryRequirements;
		vkGetBufferMemoryRequirements (graphics_device, Buffers[b], &MemoryRequirements);

		MemoryAllocateInfo.allocationSize += MemoryRequirements.size;
		GetMemoryTypeIndex (MemoryRequirements, physical_device_memory_properties, RequiredTypes, &MemoryAllocateInfo.memoryTypeIndex);
	}

	if (vkAllocateMemory (graphics_device, &MemoryAllocateInfo, NULL, OutBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	for (uint32_t b = 0; b < BufferCount; b++)
	{
		if (vkBindBufferMemory (graphics_device, Buffers[b], *OutBufferMemory, Offsets[b]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		}
	}

	my_free (Offsets);

	return 0;
}

int MapDataToBuffer (VkDevice graphics_device,
	VkDeviceMemory Memory,
	VkDeviceSize Offset,
	VkDeviceSize Size,
	void* DataSource)
{
	void* Data;
	if (vkMapMemory (graphics_device, Memory, Offset, Size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, DataSource, (size_t)Size);
	vkUnmapMemory (graphics_device, Memory);

	return 0;
}

int CreateImage (
	VkDevice graphics_device,
	uint32_t graphics_queue_family_index,
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
	ImageCreateInfo.pQueueFamilyIndices = &graphics_queue_family_index;

	if (vkCreateImage (graphics_device, &ImageCreateInfo, NULL, OutImage) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
	}

	return 0;
}

int AllocateBindImageMemory (VkDevice graphics_device, VkImage* Images, uint32_t ImageCount, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkMemoryPropertyFlags RequiredTypes, VkDeviceMemory* OutMemory)
{
	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkDeviceSize* Offsets = (VkDeviceSize*)my_calloc (ImageCount, sizeof (VkDeviceSize));

	for (uint32_t i = 0; i < ImageCount; i++)
	{
		VkMemoryRequirements MemoryRequirements = { 0 };
		vkGetImageMemoryRequirements (graphics_device, Images[i], &MemoryRequirements);

		MemoryAllocateInfo.allocationSize += MemoryRequirements.size;

		GetMemoryTypeIndex (MemoryRequirements, physical_device_memory_properties, RequiredTypes, &MemoryAllocateInfo.memoryTypeIndex);
	}

	if (vkAllocateMemory (graphics_device, &MemoryAllocateInfo, NULL, OutMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	for (uint32_t i = 0; i < ImageCount; i++)
	{
		if (vkBindImageMemory (graphics_device, Images[i], *OutMemory, Offsets[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		}
	}

	my_free (Offsets);

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

	*FileContents = (char*)my_malloc (sizeof (uint32_t) * FileSize);
	fread (*FileContents, sizeof (uint32_t), FileSize, VertFile);
	fclose (VertFile);

	return 0;
}

int ChangeImageLayout ()
{
	return 0;
}

int CopyBufferToBuffer (VkDevice graphics_device,
	VkCommandPool CommandPool,
	VkQueue graphics_queue,
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

	if (vkAllocateCommandBuffers (graphics_device, &CommandBufferAllocateInfo, &CommandBuffer) != VK_SUCCESS)
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

	SubmitOneTimeCmd (graphics_queue, CommandBuffer);

	vkFreeCommandBuffers (graphics_device, CommandPool, 1, &CommandBuffer);

	return 0;
}

int CopyBufferToImage ()
{
	return 0;
}

int CreateShader (char* FullFilePath,
	VkDevice graphics_device,
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

	FileContents = (char*)my_malloc (FileSize);
	fread (FileContents, sizeof (uint32_t), FileSize, VertFile);
	fclose (VertFile);

	VkShaderModuleCreateInfo ShaderModuleCreateInfo;
	memset (&ShaderModuleCreateInfo, 0, sizeof (VkShaderModuleCreateInfo));

	ShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ShaderModuleCreateInfo.pCode = (uint32_t*)FileContents;
	ShaderModuleCreateInfo.codeSize = FileSize;

	if (vkCreateShaderModule (graphics_device, &ShaderModuleCreateInfo, NULL, ShaderModule) != VK_SUCCESS)
	{
		my_free (FileContents);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	my_free (FileContents);

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
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties,
	VkMemoryPropertyFlags RequiredMemoryTypes,
	uint32_t* MemoryTypeIndex)
{
	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << i) && RequiredMemoryTypes & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			*MemoryTypeIndex = i;
			break;
		}
	}

	return 0;
}

int SubmitOneTimeCmd (VkQueue graphics_queue, VkCommandBuffer CommandBuffer)
{
	VkSubmitInfo SubmitInfo = { 0 };

	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffer;

	if (vkQueueSubmit (graphics_queue, 1, &SubmitInfo, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	vkQueueWaitIdle (graphics_queue);

	return 0;
}

void DestroyBufferAndBufferMemory (VkDevice graphics_device,
	VkBuffer Buffer,
	VkDeviceMemory BufferMemory)
{
	if (Buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (graphics_device, Buffer, NULL);
	}

	if (BufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory (graphics_device, BufferMemory, NULL);
	}
}