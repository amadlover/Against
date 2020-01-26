#include "graphics_utils.h"
#include "error.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

int create_buffer (VkDevice graphics_device,
	VkDeviceSize size,
	VkBufferUsageFlags Usage,
	VkSharingMode SharingMode,
	uint32_t graphics_queue_family_index,
	VkBuffer* OutBuffer)
{
	VkBufferCreateInfo create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = size;
	create_info.queueFamilyIndexCount = 1;
	create_info.pQueueFamilyIndices = &graphics_queue_family_index;
	create_info.usage = Usage;

	if (vkCreateBuffer (graphics_device, &create_info, NULL, OutBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	return 0;
}

int allocate_bind_buffer_memory (VkDevice graphics_device,
	VkBuffer* Buffers,
	uint32_t BufferCount,
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties,
	VkMemoryPropertyFlags RequiredTypes,
	VkDeviceMemory* OutBufferMemory)
{
	VkMemoryAllocateInfo memory_allocation = { 0 };
	memory_allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkDeviceSize* Offsets = (VkDeviceSize*)my_calloc (BufferCount, sizeof (VkDeviceSize));

	for (uint32_t b = 0; b < BufferCount; b++)
	{
		Offsets[b] = memory_allocation.allocationSize;

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements (graphics_device, Buffers[b], &memory_requirements);

		memory_allocation.allocationSize += memory_requirements.size;
		get_memory_type_index (memory_requirements, physical_device_memory_properties, RequiredTypes, &memory_allocation.memoryTypeIndex);
	}

	if (vkAllocateMemory (graphics_device, &memory_allocation, NULL, OutBufferMemory) != VK_SUCCESS)
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

int map_data_to_buffer (VkDevice graphics_device,
	VkDeviceMemory Memory,
	VkDeviceSize offset,
	VkDeviceSize size,
	void* DataSource)
{
	void* Data;
	if (vkMapMemory (graphics_device, Memory, offset, size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, DataSource, (size_t)size);
	vkUnmapMemory (graphics_device, Memory);

	return 0;
}

int create_image (
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

int allocate_bind_image_memory (VkDevice graphics_device, VkImage* Images, uint32_t ImageCount, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkMemoryPropertyFlags RequiredTypes, VkDeviceMemory* OutMemory)
{
	VkMemoryAllocateInfo MemoryAllocateInfo = { 0 };

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkDeviceSize* Offsets = (VkDeviceSize*)my_calloc (ImageCount, sizeof (VkDeviceSize));

	for (uint32_t i = 0; i < ImageCount; i++)
	{
		VkMemoryRequirements MemoryRequirements = { 0 };
		vkGetImageMemoryRequirements (graphics_device, Images[i], &MemoryRequirements);

		MemoryAllocateInfo.allocationSize += MemoryRequirements.size;

		get_memory_type_index (MemoryRequirements, physical_device_memory_properties, RequiredTypes, &MemoryAllocateInfo.memoryTypeIndex);
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

int change_image_layout ()
{
	return 0;
}

int copy_buffer_to_buffer (VkDevice graphics_device,
	VkCommandPool CommandPool,
	VkQueue graphics_queue,
	VkBuffer SrcBuffer,
	VkBuffer DstBuffer,
	VkDeviceSize size)
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
	BufferCopy.size = size;

	vkCmdCopyBuffer (CommandBuffer, SrcBuffer, DstBuffer, 1, &BufferCopy);

	vkEndCommandBuffer (CommandBuffer);

	submit_one_time_cmd (graphics_queue, CommandBuffer);

	vkFreeCommandBuffers (graphics_device, CommandPool, 1, &CommandBuffer);

	return 0;
}

int copy_buffer_to_image ()
{
	return 0;
}

int create_shader (const char* FullFilePath,
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

int get_memory_type_index (VkMemoryRequirements MemoryRequirements,
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

int submit_one_time_cmd (VkQueue graphics_queue, VkCommandBuffer CommandBuffer)
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

void destroy_buffer_and_buffer_memory (VkDevice graphics_device,
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