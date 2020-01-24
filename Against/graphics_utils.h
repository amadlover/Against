#pragma once

#include <vulkan/vulkan.h>

int CreateBuffer (VkDevice graphics_device, VkDeviceSize Size, VkBufferUsageFlags Usage, VkSharingMode SharingMode, uint32_t graphics_queue_family_index, VkBuffer* OutBuffer);
int AllocateBindBufferMemory (VkDevice graphics_device, VkBuffer* Buffers, uint32_t BufferCount, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkMemoryPropertyFlags RequiredTypes, VkDeviceMemory* OutMemory);
int MapDataToBuffer (VkDevice graphics_device, VkDeviceMemory Memory, VkDeviceSize Offset, VkDeviceSize Size, void* DataSource);

int CreateImage (VkDevice graphics_device, uint32_t graphics_queue_family_index, VkExtent3D Extent, uint32_t ArrayLayers, VkFormat Format, VkImageLayout InitialLayout, VkSharingMode SharingMode, VkImage* OutImage);
int AllocateBindImageMemory (VkDevice graphics_device, VkImage* Images, uint32_t ImageCount, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkMemoryPropertyFlags RequiredTypes, VkDeviceMemory* OutMemory);

int ChangeImageLayout ();

int CopyBufferToBuffer (VkDevice graphics_device, VkCommandPool CommandPool, VkQueue graphics_queue, VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size);
int CopyBufferToImage ();

int CreateShader (char* FilePath, VkDevice graphics_device, VkShaderStageFlagBits ShaderStage, VkShaderModule* ShaderModule, VkPipelineShaderStageCreateInfo* ShaderStageCreateInfo);

int GetMemoryTypeIndex (VkMemoryRequirements MemoryRequirements, VkPhysicalDeviceMemoryProperties physical_device_memory_properties, VkMemoryPropertyFlags RequiredMemoryTypes, uint32_t* MemoryTypeIndex);
int SubmitOneTimeCmd (VkQueue graphics_queue, VkCommandBuffer CommandBuffer);

void DestroyBufferAndBufferMemory (VkDevice graphics_device, VkBuffer Buffer, VkDeviceMemory BufferMemory);