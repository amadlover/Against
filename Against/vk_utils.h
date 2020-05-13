#pragma once

#include "error.h"
#include <vulkan/vulkan.h>

typedef struct _vk_command_pool
{
    VkCommandPool command_pool;
    VkCommandBuffer* command_buffers;
    VkSemaphore* submit_signal_semaphores;
    size_t command_buffers_count;
} vk_command_pool;

AGAINST_RESULT vk_utils_create_buffer (
    VkDevice graphics_device, 
    VkDeviceSize size, 
    VkBufferUsageFlags usage, 
    VkSharingMode sharing_mode, 
    uint32_t graphics_queue_family_index, 
    VkBuffer* out_buffer
);

AGAINST_RESULT vk_utils_allocate_bind_buffer_memory (
    VkDevice graphics_device, 
    VkBuffer* buffers, 
    uint32_t buffer_count, 
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties, 
    VkMemoryPropertyFlags required_types, 
    VkDeviceMemory* out_memory
);

AGAINST_RESULT vk_utils_map_data_to_device_memory (
    VkDevice graphics_device, 
    VkDeviceMemory memory, 
    VkDeviceSize offset, 
    VkDeviceSize size, 
    void* data_source
);

AGAINST_RESULT vk_utils_create_image (
    VkDevice graphics_device, 
    uint32_t src_queue_family_index, 
    VkExtent3D extent, 
    uint32_t array_layers, 
    VkFormat format, 
    VkImageLayout initial_layout, 
    VkSharingMode sharing_mode, 
    VkImageUsageFlags usage,
    VkImage* out_image
);

AGAINST_RESULT vk_utils_create_image_view (
    VkDevice graphics_device,
    VkImage image,
    VkImageView* out_image_view
);

AGAINST_RESULT vk_utils_allocate_bind_image_memory (
    VkDevice graphics_device, 
    VkImage* images, 
    uint32_t image_count, 
    VkPhysicalDeviceMemoryProperties physical_device_memory_properties, 
    VkMemoryPropertyFlags required_types, 
    VkDeviceMemory* out_memory
);

AGAINST_RESULT vk_utils_change_image_layout (
    VkDevice graphics_device,
    VkQueue transfer_queue,
    vk_command_pool transfer_command_pool,
    uint32_t src_queue_family_index,
    uint32_t dst_queue_family_index,
    VkImage image,
    uint32_t layer_count,
    VkImageLayout old_layout,
    VkImageLayout new_layout,
    VkAccessFlags src_access,
    VkAccessFlags dst_access,
    VkPipelineStageFlags src_stage,
    VkPipelineStageFlags dst_stage
);

AGAINST_RESULT vk_utils_copy_buffer_to_buffer (
    VkDevice graphics_device, 
    vk_command_pool transfer_command_pool, 
    VkQueue transfer_queue, 
    VkBuffer src_buffer, 
    VkBuffer dst_buffer, 
    VkDeviceSize size
);

AGAINST_RESULT vk_utils_copy_buffer_to_image (
    VkDevice graphics_device,
    vk_command_pool transfer_command_pool,
    VkQueue transfer_queue,
    VkDeviceSize offset,
    VkBuffer buffer,
    VkImage* image,
    VkExtent3D extent,
    uint32_t layer_count
);

AGAINST_RESULT vk_utils_create_shader (
    const char* file_path, 
    VkDevice graphics_device, 
    VkShaderStageFlagBits shader_stage, 
    VkShaderModule* shader_module, 
    VkPipelineShaderStageCreateInfo* shader_stage_create_info
);

AGAINST_RESULT vk_utils_create_descriptor_pool (
    VkDevice graphics_device,
    VkDescriptorType* types,
    size_t* type_counts,
    size_t num_types,
    size_t max_sets,
    VkDescriptorPool* out_descriptor_pool
);

AGAINST_RESULT vk_utils_create_descriptor_set_layout_bindings (
    VkDevice graphics_device,
    VkDescriptorType* descriptor_types,
    size_t* descriptor_count_per_type,
    size_t* bindings,
    size_t num_descriptor_types,
    VkShaderStageFlags* stage_flags,
    VkDescriptorSetLayoutBinding* out_descriptor_set_layout_bindings
);

AGAINST_RESULT vk_utils_create_descriptor_set_layout (
    VkDevice graphics_device,
    VkDescriptorSetLayoutBinding* bindings,
    size_t num_bindings,
    VkDescriptorSetLayout* out_descriptor_set_layout
);

AGAINST_RESULT vk_utils_allocate_descriptor_sets (
    VkDevice graphics_device,
    VkDescriptorPool descriptor_pool,
    VkDescriptorSetLayout* descriptor_set_layouts,
    size_t num_descriptor_sets,
    VkDescriptorSet* out_descriptor_sets
);

AGAINST_RESULT vk_utils_update_descriptor_sets (
    VkDevice graphics_device,
    VkDescriptorSet* descriptor_sets,
    VkDescriptorType* descriptor_types,
    size_t* descriptor_counts,
    size_t* binding_numbers,
    VkDescriptorBufferInfo* buffer_infos,
    VkDescriptorImageInfo* image_infos,
    size_t num_descriptor_sets
);

AGAINST_RESULT vk_utils_create_command_pools (
    VkDevice graphics_device, 
    size_t queue_family_index, 
    size_t command_pools_count,
    VkCommandPoolCreateFlags flags, 
    vk_command_pool* out_command_pools
);

AGAINST_RESULT vk_utils_allocate_command_buffers (
    VkDevice graphics_device,
    VkCommandBufferLevel level, 
    size_t command_pools_count, 
    size_t* command_buffers_counts, 
    vk_command_pool* in_out_command_pools
);

AGAINST_RESULT vk_utils_create_semaphores (
    VkDevice graphics_device, 
    size_t semaphores_count, 
    VkSemaphore* out_semaphores
);

AGAINST_RESULT vk_utils_create_semaphores_for_command_pools (
    VkDevice graphics_device,
    size_t command_pools_count,
    vk_command_pool* in_out_command_pools
);

void vk_utils_get_aligned_size (size_t original_size, 
    size_t alignment, 
    size_t* out_aligned_size);

void vk_utils_destroy_buffer_and_buffer_memory (
    VkDevice graphics_device, 
    VkBuffer buffer, 
    VkDeviceMemory buffer_memory
);

void vk_utils_destroy_command_pools_and_buffers (
    VkDevice graphics_device,
    vk_command_pool* command_pools,
    size_t command_pools_count
);