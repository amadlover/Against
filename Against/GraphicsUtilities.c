#include "GraphicsUtilities.h"
#include "Error.h"
#include "Graphics.h"

int CreateUniformBuffer (VkBuffer* UniformBuffer, VkDeviceMemory* UniformBufferMemory, VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags RequiredMemoryTypes)
{
	OutputDebugString (L"CreateUniformBuffer\n");

	VkBufferCreateInfo CreateInfo;
	memset (&CreateInfo, 0, sizeof (VkBufferCreateInfo));

	CreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	CreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	CreateInfo.size = Size;
	CreateInfo.usage = Usage;

	if (vkCreateBuffer (GraphicsDevice, &CreateInfo, NULL, UniformBuffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements (GraphicsDevice, *UniformBuffer, &MemoryRequirements);

	VkMemoryAllocateInfo MemoryAllocateInfo;
	memset (&MemoryAllocateInfo, 0, sizeof (VkMemoryAllocateInfo));

	MemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocateInfo.allocationSize = MemoryRequirements.size;

	uint32_t RequiredTypes = RequiredMemoryTypes;// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	for (uint32_t i = 0; i < PhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if (MemoryRequirements.memoryTypeBits & (1 << i) && RequiredTypes & PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags)
		{
			MemoryAllocateInfo.memoryTypeIndex = i;
			break;
		}
	}

	if (vkAllocateMemory (GraphicsDevice, &MemoryAllocateInfo, NULL, UniformBufferMemory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	if (vkBindBufferMemory (GraphicsDevice, *UniformBuffer, *UniformBufferMemory, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
	}

	return 0;
}

int CreateShader (char* FilePath, VkShaderStageFlags ShaderStage, VkPipelineShaderStageCreateInfo* ShaderStageCreateInfo)
{
	return 0;
}

int CreateTextureOnDevice (char* FilePath, VkImage* TextureImage, VkDeviceMemory* TextureImageMemory)
{
	return 0;
}