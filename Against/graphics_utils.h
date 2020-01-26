#pragma once

#include <vulkan/vulkan.h>

int create_buffer (VkDevice graphics_device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharing_mode, uint32_t graphics_queue_family_index, VkBuffer* out_buffer);
int allocate_bind_buffer_memory (VkDevice graphics_device, VkBuffer* buffers, uint32_t buffer_count, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkMemoryPropertyFlags required_types, VkDeviceMemory* out_memory);
int map_data_to_buffer (VkDevice graphics_device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, void* data_source);

int create_image (VkDevice graphics_device, uint32_t graphics_queue_family_index, VkExtent3D extent, uint32_t array_layers, VkFormat format, VkImageLayout initial_layout, VkSharingMode sharing_mode, VkImage* out_image);
int allocate_bind_image_memory (VkDevice graphics_device, VkImage* images, uint32_t image_count, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkMemoryPropertyFlags required_types, VkDeviceMemory* out_memory);

int change_image_layout ();

int copy_buffer_to_buffer (VkDevice graphics_device, VkCommandPool command_pool, VkQueue graphics_queue, VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size);
int copy_buffer_to_image ();

int create_shader (const char* file_path, VkDevice graphics_device, VkShaderStageFlagBits shader_stage, VkShaderModule* shader_module, VkPipelineShaderStageCreateInfo* shader_stage_create_info);

int get_memory_type_index (VkMemoryRequirements memory_requirements, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkMemoryPropertyFlags required_types, uint32_t* memory_type_index);
int submit_one_time_cmd (VkQueue graphics_queue, VkCommandBuffer command_buffer);

void destroy_buffer_and_buffer_memory (VkDevice graphics_device, VkBuffer buffer, VkDeviceMemory buffer_memory);