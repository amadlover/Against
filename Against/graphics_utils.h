#pragma once

#include <vulkan/vulkan.h>

int graphics_utils_create_buffer (
    VkDevice graphics_device, 
    VkDeviceSize size, 
    VkBufferUsageFlags usage, 
    VkSharingMode sharing_mode, 
    uint32_t graphics_queue_family_index, 
    VkBuffer* out_buffer
);

int graphics_utils_allocate_bind_buffer_memory (
    VkDevice graphics_device, 
    VkBuffer* buffers, 
    uint32_t buffer_count, 
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties, 
    VkMemoryPropertyFlags required_types, 
    VkDeviceMemory* out_memory
);

int graphics_utils_map_data_to_device_memory (
    VkDevice graphics_device, 
    VkDeviceMemory memory, 
    VkDeviceSize offset, 
    VkDeviceSize size, 
    void* data_source
);

int graphics_utils_create_image (
    VkDevice graphics_device, 
    uint32_t graphics_queue_family_index, 
    VkExtent3D extent, 
    uint32_t array_layers, 
    VkFormat format, 
    VkImageLayout initial_layout, 
    VkSharingMode sharing_mode, 
    VkImageUsageFlags usage,
    VkImage* out_image
);

int graphics_utils_create_image_view (
    VkDevice graphics_device,
    VkImage image,
    VkImageView* out_image_view
);

int graphics_utils_allocate_bind_image_memory (
    VkDevice graphics_device, 
    VkImage* images, 
    uint32_t image_count, 
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties, 
    VkMemoryPropertyFlags required_types, 
    VkDeviceMemory* out_memory
);

int graphics_utils_change_image_layout (
    VkDevice graphics_device,
    VkQueue graphics_queue,
    VkCommandPool command_pool,
    uint32_t graphics_queue_family_index,
    VkImage image,
    uint32_t layer_count,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    VkAccessFlags src_access,
    VkAccessFlags dst_access,
    VkPipelineStageFlags src_stage,
    VkPipelineStageFlags dst_stage
);

int graphics_utils_copy_buffer_to_buffer (
    VkDevice graphics_device, 
    VkCommandPool command_pool, 
    VkQueue graphics_queue, 
    VkBuffer src_buffer, 
    VkBuffer dst_buffer, 
    VkDeviceSize size
);

int graphics_utils_copy_buffer_to_image (
    VkDevice graphics_device,
    VkCommandPool command_pool,
    VkQueue graphics_queue,
    VkDeviceSize offset,
    VkBuffer buffer,
    VkImage* image,
    VkExtent3D extent,
    uint32_t layer_count
);

int graphics_utils_create_shader (
    const char* file_path, 
    VkDevice graphics_device, 
    VkShaderStageFlagBits shader_stage, 
    VkShaderModule* shader_module, 
    VkPipelineShaderStageCreateInfo* shader_stage_create_info
);

void graphics_utils_destroy_buffer_and_buffer_memory (
    VkDevice graphics_device, 
    VkBuffer buffer, 
    VkDeviceMemory buffer_memory
);