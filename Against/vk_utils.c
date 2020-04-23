#include "vk_utils.h"
#include "error.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

int vk_utils_create_buffer (VkDevice graphics_device,
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

int get_one_time_command_buffer (VkDevice graphics_device, VkCommandPool common_command_pool, VkCommandBuffer* out_buffer)
{
	VkCommandBufferAllocateInfo command_buffer_allocate_info = { 0 };

	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = common_command_pool;
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

	*out_buffer = command_buffer;

	return 0;
}


int vk_utils_allocate_bind_buffer_memory (VkDevice graphics_device,
	VkBuffer* buffers,
	uint32_t buffer_count,
	VkPhysicalDeviceMemoryProperties physical_device_memory_properties,
	VkMemoryPropertyFlags required_types,
	VkDeviceMemory* out_buffer_memory)
{
	VkMemoryAllocateInfo memory_allocation = { 0 };
	memory_allocation.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkDeviceSize* offsets = (VkDeviceSize*)utils_my_calloc (buffer_count, sizeof (VkDeviceSize));

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

	utils_my_free (offsets);

	return 0;
}

int vk_utils_map_data_to_device_memory (VkDevice graphics_device,
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

int vk_utils_create_image (
	VkDevice graphics_device,
	uint32_t graphics_queue_family_index,
	VkExtent3D extent,
	uint32_t array_layers,
	VkFormat format,
	VkImageLayout initial_layout,
	VkSharingMode sharing_mode,
	VkImageUsageFlags usage,
	VkImage* out_image
)
{
	VkImageCreateInfo create_info = { 0 };

	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	create_info.arrayLayers = array_layers;
	create_info.extent = extent;
	create_info.format = format;
	create_info.mipLevels = 1;
	create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	create_info.initialLayout = initial_layout;
	create_info.sharingMode = sharing_mode;
	create_info.queueFamilyIndexCount = 1;
	create_info.pQueueFamilyIndices = &graphics_queue_family_index;
	create_info.usage = usage;
	create_info.imageType = VK_IMAGE_TYPE_2D;

	if (vkCreateImage (graphics_device, &create_info, NULL, out_image) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE;
	}

	return 0;
}

int vk_utils_create_image_view (
	VkDevice graphics_device,
	VkImage image,
	VkImageView* out_image_view
)
{
	VkComponentMapping components = { 0 };
	VkImageSubresourceRange subresource_range = { 0 };
	subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range.layerCount = 1;
	subresource_range.levelCount = 1;

	VkImageViewCreateInfo create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	create_info.image = image;
	create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
	create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	create_info.components = components;
	create_info.subresourceRange = subresource_range;	

	if (vkCreateImageView (graphics_device, &create_info, NULL, out_image_view) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_IMAGE_VIEW;
	}

	return 0;
}

int vk_utils_allocate_bind_image_memory (
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

	VkDeviceSize* offsets = (VkDeviceSize*)utils_my_calloc (image_count, sizeof (VkDeviceSize));

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

	utils_my_free (offsets);

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

	*file_contents = (char*)utils_my_malloc (sizeof (uint32_t) * file_size);
	fread (*file_contents, sizeof (uint32_t), file_size, vert_file);
	fclose (vert_file);

	return 0;
}

int vk_utils_change_image_layout (
	VkDevice graphics_device,
	VkQueue graphics_queue,
	VkCommandPool common_command_pool,
	uint32_t graphics_queue_family_index,
	VkImage image,
	uint32_t layer_count,
	VkImageLayout old_layout,
	VkImageLayout new_layout,
	VkAccessFlags src_access,
	VkAccessFlags dst_access,
	VkPipelineStageFlags src_stage,
	VkPipelineStageFlags dst_stage)
{
	AGAINSTRESULT result = AGAINST_SUCCESS;

	VkCommandBufferAllocateInfo allocate_info = { 0 };
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandPool = common_command_pool;
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;

	if (vkAllocateCommandBuffers (graphics_device, &allocate_info, &command_buffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_COMMAND_BUFFER;
	}

	VkCommandBufferBeginInfo begin_info = { 0 };
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if (vkBeginCommandBuffer (command_buffer, &begin_info) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_BEGIN_COMMAND_BUFFER;
	}

	VkImageSubresourceRange subresource_range = { 0 };
	subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range.baseMipLevel = 0;
	subresource_range.levelCount = 1;
	subresource_range.baseArrayLayer = 0;
	subresource_range.layerCount = layer_count;

	VkImageMemoryBarrier image_memory_barrier = { 0 };
	image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	image_memory_barrier.image = image;
	image_memory_barrier.srcQueueFamilyIndex = graphics_queue_family_index;
	image_memory_barrier.dstQueueFamilyIndex = graphics_queue_family_index;
	image_memory_barrier.oldLayout = old_layout;
	image_memory_barrier.newLayout = new_layout;
	image_memory_barrier.srcAccessMask = src_access;
	image_memory_barrier.dstAccessMask = dst_access;
	image_memory_barrier.subresourceRange = subresource_range;

	vkCmdPipelineBarrier (command_buffer, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &image_memory_barrier);
	if (vkEndCommandBuffer (command_buffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER;
	}
	CHECK_AGAINST_RESULT (submit_one_time_cmd (graphics_queue, command_buffer), result);
	vkFreeCommandBuffers (graphics_device, common_command_pool, 1, &command_buffer);

	return 0;
}

int vk_utils_copy_buffer_to_buffer (
	VkDevice graphics_device,
	VkCommandPool common_command_pool,
	VkQueue graphics_queue,
	VkBuffer src_buffer,
	VkBuffer dst_buffer,
	VkDeviceSize size)
{
	AGAINSTRESULT result;

	VkCommandBufferAllocateInfo command_buffer_allocate_info = { 0 };

	command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	command_buffer_allocate_info.commandPool = common_command_pool;
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
	if (vkEndCommandBuffer (command_buffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER;
	}
	CHECK_AGAINST_RESULT (submit_one_time_cmd (graphics_queue, command_buffer), result);
	vkFreeCommandBuffers (graphics_device, common_command_pool, 1, &command_buffer);

	return 0;
}

int vk_utils_copy_buffer_to_image (
	VkDevice graphics_device,
	VkCommandPool common_command_pool,
	VkQueue graphics_queue,
	VkDeviceSize offset,
	VkBuffer buffer,
	VkImage* image,
	VkExtent3D extent,
	uint32_t layer_count)
{
	AGAINSTRESULT result;
	VkCommandBuffer command_buffer;
	CHECK_AGAINST_RESULT (get_one_time_command_buffer (graphics_device, common_command_pool, &command_buffer), result);

	VkImageSubresourceLayers subresource_layers = { 0 };
	subresource_layers.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_layers.layerCount = layer_count;

	VkBufferImageCopy buffer_image_copy = { 0 };
	buffer_image_copy.bufferOffset = offset;
	buffer_image_copy.imageExtent = extent;
	buffer_image_copy.imageSubresource = subresource_layers;

	vkCmdCopyBufferToImage (command_buffer, buffer, *image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_image_copy);
	if (vkEndCommandBuffer (command_buffer) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_END_COMMAND_BUFFER;
	}

	CHECK_AGAINST_RESULT (submit_one_time_cmd (graphics_queue, command_buffer), result);
	vkFreeCommandBuffers (graphics_device, common_command_pool, 1, &command_buffer);

	return 0;
}

int vk_utils_create_shader (const char* full_file_path,
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
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

	fseek (vert_file, 0, SEEK_END);

	uint32_t file_size = (uint32_t)ftell (vert_file);
	rewind (vert_file);

	file_contents = (char*)utils_my_malloc (file_size);
	fread (file_contents, sizeof (uint32_t), file_size, vert_file);
	fclose (vert_file);

	VkShaderModuleCreateInfo shader_module_create_info = { 0 };

	shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shader_module_create_info.pCode = (uint32_t*)file_contents;
	shader_module_create_info.codeSize = file_size;

	utils_my_free (file_contents);
	
	if (vkCreateShaderModule (graphics_device, &shader_module_create_info, NULL, shader_module) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_SHADER_MODULE;
	}

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

int vk_utils_create_descriptor_pool (
	VkDevice graphics_device,
	VkDescriptorType* types,
	size_t* type_counts,
	size_t num_types,
	size_t max_sets,
	VkDescriptorPool* out_descriptor_pool)
{
	VkDescriptorPoolSize* pool_sizes = (VkDescriptorPoolSize*) utils_my_calloc (num_types, sizeof (VkDescriptorPoolSize));

	for (size_t ps = 0; ps < num_types; ++ps)
	{
		pool_sizes[ps].type = types[ps];
		pool_sizes[ps].descriptorCount = type_counts[ps];
	}

	VkDescriptorPoolCreateInfo create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	create_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	create_info.maxSets = max_sets;
	create_info.poolSizeCount = num_types;
	create_info.pPoolSizes = pool_sizes;

	if (vkCreateDescriptorPool (graphics_device, &create_info, NULL, out_descriptor_pool) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_POOL;
	}

	utils_my_free (pool_sizes);

	return 0;
}

int vk_utils_create_descriptor_set_layout_bindings (
	VkDevice graphics_device,
	VkDescriptorType* descriptor_types,
	size_t* descriptor_count_per_type,
	size_t* bindings,
	size_t num_descriptor_types,
	VkShaderStageFlags* stage_flags,
	VkDescriptorSetLayoutBinding* out_descriptor_set_layout_bindings)
{
	for (size_t d = 0; d < num_descriptor_types; ++d)
	{
		out_descriptor_set_layout_bindings[d].binding = bindings[d];
		out_descriptor_set_layout_bindings[d].descriptorType = descriptor_types[d];
		out_descriptor_set_layout_bindings[d].descriptorCount = descriptor_count_per_type[d];
		out_descriptor_set_layout_bindings[d].stageFlags = stage_flags[d];
	}

	return 0;
}

int vk_utils_create_descriptor_set_layout (
	VkDevice graphics_device,
	VkDescriptorSetLayoutBinding* bindings,
	size_t num_bindings,
	VkDescriptorSetLayout* out_descriptor_set_layout)
{
	VkDescriptorSetLayoutCreateInfo create_info = { 0 };
	create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	create_info.bindingCount = num_bindings;
	create_info.pBindings = bindings;

	if (vkCreateDescriptorSetLayout (graphics_device, &create_info, NULL, out_descriptor_set_layout) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_CREATE_DESCRIPTOR_SET_LAYOUT;
	}

	return 0;
}

int vk_utils_allocate_descriptor_sets (
	VkDevice graphics_device,
	VkDescriptorPool descriptor_pool,
	VkDescriptorSetLayout* descriptor_set_layouts,
	size_t num_descriptor_sets,
	VkDescriptorSet* out_descriptor_sets)
{
	VkDescriptorSetAllocateInfo allocate_info = { 0 };
	allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocate_info.descriptorPool = descriptor_pool;
	allocate_info.descriptorSetCount = num_descriptor_sets;
	allocate_info.pSetLayouts = descriptor_set_layouts;

	if (vkAllocateDescriptorSets (graphics_device, &allocate_info, out_descriptor_sets) != VK_SUCCESS)
	{
		return AGAINST_ERROR_GRAPHICS_ALLOCATE_DESCRIPTOR_SET;
	}

	return 0;
}

int vk_utils_update_descriptor_sets (
	VkDevice graphics_device,
	VkDescriptorSet* descriptor_sets,
	VkDescriptorType* descriptor_types,
	size_t* descriptor_counts,
	size_t* binding_numbers,
	VkDescriptorBufferInfo* buffer_infos,
	VkDescriptorImageInfo* image_infos,
	size_t num_descriptor_sets
)
{
	VkWriteDescriptorSet* descriptor_writes = (VkWriteDescriptorSet*)utils_my_calloc (num_descriptor_sets, sizeof (VkWriteDescriptorSet));
	for (size_t ds = 0; ds < num_descriptor_sets; ++ds)
	{
		descriptor_writes[ds].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptor_writes[ds].descriptorCount = descriptor_counts[ds];
		descriptor_writes[ds].dstSet = descriptor_sets[ds];
		descriptor_writes[ds].dstBinding = binding_numbers[ds];
		descriptor_writes[ds].pBufferInfo = buffer_infos + ds;
		descriptor_writes[ds].pImageInfo = image_infos + ds;
	}

	vkUpdateDescriptorSets (graphics_device, num_descriptor_sets, descriptor_writes, 0, NULL);

	utils_my_free (descriptor_writes);

	return 0;
}

void vk_utils_destroy_buffer_and_buffer_memory (VkDevice graphics_device,
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