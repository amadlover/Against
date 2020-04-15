#include "graphics_utils.h"
#include "error.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

int graphics_utils_create_buffer (VkDevice graphics_device,
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkSharingMode sharing_mode,
	uint32_t graphics_queue_family_index,
	VkBuffer* out_buffer)
{
	VkBufferCreateInfo create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	create_info.size = size;
	create_info.queueFamilyIndexCount = 1;
	create_info.pQueueFamilyIndices = &graphics_queue_family_index;
	create_info.usage = usage;

	if (vkCreateBuffer (graphics_device, &create_info, NULL, out_buffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_BUFFER;
	}

	return 0;
}

int get_memory_type_index (
	VkMemoryRequirements memory_requirements,
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties,
	VkMemoryPropertyFlags required_memory_types,
	uint32_t* memory_type_index
)
{
	for (uint32_t i = 0; i < physical_device_memory_properties.memoryTypeCount; i++)
	{
		if (memory_requirements.memoryTypeBits & (1 << i) && required_memory_types & physical_device_memory_properties.memoryTypes[i].propertyFlags)
		{
			*memory_type_index = i;
			break;
		}
	}

	return 0;
}

int submit_one_time_cmd (VkQueue graphics_queue, VkCommandBuffer command_buffer)
{
	VkSubmitInfo submit_info = { 0 };

	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	if (vkQueueSubmit (graphics_queue, 1, &submit_info, 0) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_QUEUE_SUBMIT;
	}

	vkQueueWaitIdle (graphics_queue);

	return 0;
}

int graphics_utils_allocate_bind_buffer_memory (VkDevice graphics_device,
	VkBuffer* buffers,
	uint32_t buffer_count,
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties,
	VkMemoryPropertyFlags required_types,
	VkDeviceMemory* out_buffer_memory)
{
	VkMemoryAllocateInfo memory_allocation = { 0 };
	memory_allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkDeviceSize* offsets = (VkDeviceSize*)my_calloc (buffer_count, sizeof (VkDeviceSize));

	for (uint32_t b = 0; b < buffer_count; b++)
	{
		offsets[b] = memory_allocation.allocationSize;

		VkMemoryRequirements memory_requirements;
		vkGetBufferMemoryRequirements (graphics_device, buffers[b], &memory_requirements);

		memory_allocation.allocationSize += memory_requirements.size;
		get_memory_type_index (memory_requirements, physical_device_memory_properties, required_types, &memory_allocation.memoryTypeIndex);
	}

	if (vkAllocateMemory (graphics_device, &memory_allocation, NULL, out_buffer_memory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	for (uint32_t b = 0; b < buffer_count; b++)
	{
		if (vkBindBufferMemory (graphics_device, buffers[b], *out_buffer_memory, offsets[b]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_BUFFER_MEMORY;
		}
	}

	my_free (offsets);

	return 0;
}

int graphics_utils_map_data_to_device_memory (VkDevice graphics_device,
	VkDeviceMemory memory,
	VkDeviceSize offset,
	VkDeviceSize size,
	void* data_source)
{
	void* Data;
	if (vkMapMemory (graphics_device, memory, offset, size, 0, &Data) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_MAP_MEMORY;
	}

	memcpy (Data, data_source, (size_t)size);
	vkUnmapMemory (graphics_device, memory);

	return 0;
}

int graphics_utils_create_image (
	VkDevice graphics_device,
	uint32_t graphics_queue_family_index,
	VkExtent3D extent,
	uint32_t array_layers,
	VkFormat format,
	VkImageLayout initial_layout,
	VkSharingMode sharing_mode,
	VkImage* out_image
)
{
	VkImageCreateInfo create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.arrayLayers = array_layers;
	create_info.extent = extent;
	create_info.format = format;
	create_info.mipLevels = 0;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	create_info.initialLayout = initial_layout;
	create_info.sharingMode = sharing_mode;
	create_info.queueFamilyIndexCount = 1;
	create_info.pQueueFamilyIndices = &graphics_queue_family_index;

	if (vkCreateImage (graphics_device, &create_info, NULL, out_image) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
	}

	return 0;
}

int graphics_utils_allocate_bind_image_memory (
	VkDevice graphics_device, 
	VkImage* images, 
	uint32_t image_count, 
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties, 
	VkMemoryPropertyFlags required_types, 
	VkDeviceMemory* out_memory
)
{
	VkMemoryAllocateInfo memory_allocate_info = { 0 };

	memory_allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkDeviceSize* offsets = (VkDeviceSize*)my_calloc (image_count, sizeof (VkDeviceSize));

	for (uint32_t i = 0; i < image_count; i++)
	{
		VkMemoryRequirements memory_requirements = { 0 };
		vkGetImageMemoryRequirements (graphics_device, images[i], &memory_requirements);

		memory_allocate_info.allocationSize += memory_requirements.size;

		get_memory_type_index (memory_requirements, physical_device_memory_properties, required_types, &memory_allocate_info.memoryTypeIndex);
	}

	if (vkAllocateMemory (graphics_device, &memory_allocate_info, NULL, out_memory) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_MEMORY;
	}

	for (uint32_t i = 0; i < image_count; i++)
	{
		if (vkBindImageMemory (graphics_device, images[i], *out_memory, offsets[i]) != VK_SUCCESS)
		{
			return AGAINST_ERROR_GRAPHICS_BIND_IMAGE_MEMORY;
		}
	}

	my_free (offsets);

	return 0;
}

errno_t ReadShaderFile (char* full_file_path, char** file_contents)
{
	FILE* vert_file = NULL;
	errno_t Err = fopen_s (&vert_file, full_file_path, "rb");

	if (Err != 0)
	{
		return Err;
	}

	fseek (vert_file, 0, SEEK_END);

	uint32_t file_size = (uint32_t)ftell (vert_file) / sizeof (uint32_t);
	rewind (vert_file);

	*file_contents = (char*)my_malloc (sizeof (uint32_t) * file_size);
	fread (*file_contents, sizeof (uint32_t), file_size, vert_file);
	fclose (vert_file);

	return 0;
}

int graphics_utils_change_image_layout ()
{
	return 0;
}

int graphics_utils_copy_buffer_to_buffer (
	VkDevice graphics_device,
	VkCommandPool command_pool,
	VkQueue graphics_queue,
	VkBuffer src_buffer,
	VkBuffer dst_buffer,
	VkDeviceSize size)
{
	VkCommandBufferAllocateInfo command_buffer_allocate_info = { 0 };

	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = command_pool;
	command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	command_buffer_allocate_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;

	if (vkAllocateCommandBuffers (graphics_device, &command_buffer_allocate_info, &command_buffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	VkCommandBufferBeginInfo command_buffer_begin_info = { 0 };
	command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer (command_buffer, &command_buffer_begin_info) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	VkBufferCopy buffer_copy = { 0 };
	buffer_copy.size = size;

	vkCmdCopyBuffer (command_buffer, src_buffer, dst_buffer, 1, &buffer_copy);

	vkEndCommandBuffer (command_buffer);

	submit_one_time_cmd (graphics_queue, command_buffer);

	vkFreeCommandBuffers (graphics_device, command_pool, 1, &command_buffer);

	return 0;
}

int graphics_utils_copy_buffer_to_image ()
{
	return 0;
}

int graphics_utils_create_shader (const char* full_file_path,
	VkDevice graphics_device,
	VkShaderStageFlagBits shader_stage,
	VkShaderModule* shader_module,
	VkPipelineShaderStageCreateInfo* shader_stage_create_info)
{
	char* file_contents = NULL;

	FILE* vert_file = NULL;
	errno_t Err = fopen_s (&vert_file, full_file_path, "rb");

	if (Err != 0)
	{
		return Err;
	}

	fseek (vert_file, 0, SEEK_END);

	uint32_t file_size = (uint32_t)ftell (vert_file);
	rewind (vert_file);

	file_contents = (char*)my_malloc (file_size);
	fread (file_contents, sizeof (uint32_t), file_size, vert_file);
	fclose (vert_file);

	VkShaderModuleCreateInfo shader_module_create_info = { 0 };

	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.pCode = (uint32_t*)file_contents;
	shader_module_create_info.codeSize = file_size;

	if (vkCreateShaderModule (graphics_device, &shader_module_create_info, NULL, shader_module) != VK_SUCCESS)
	{
		my_free (file_contents);
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	my_free (file_contents);

	VkPipelineShaderStageCreateInfo shader_stage_c_i = { 0 };

	shader_stage_c_i.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage_c_i.stage = shader_stage;
	shader_stage_c_i.module = *shader_module;
	shader_stage_c_i.pName = "main";

	if (shader_stage == VK_SHADER_STAGE_VERTEX_BIT)
	{
		*shader_stage_create_info = shader_stage_c_i;
	}
	else if (shader_stage == VK_SHADER_STAGE_FRAGMENT_BIT)
	{
		*shader_stage_create_info = shader_stage_c_i;
	}

	return 0;
}

void graphics_utils_destroy_buffer_and_buffer_memory (VkDevice graphics_device,
	VkBuffer buffer,
	VkDeviceMemory buffer_memory)
{
	if (buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer (graphics_device, buffer, NULL);
	}

	if (buffer_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory (graphics_device, buffer_memory, NULL);
	}
}