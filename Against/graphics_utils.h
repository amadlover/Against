#pragma once

#include <vulkan/vulkan.h>

int CreateBuffer (VkDevice GraphicsDevice, VkDeviceSize Size, VkBufferUsageFlags Usage, VkSharingMode SharingMode, uint32_t GraphicsQueueFamilyIndex, VkBuffer* OutBuffer);
int AllocateBindBufferMemory (VkDevice GraphicsDevice, VkBuffer* Buffers, uint32_t BufferCount, VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, VkMemoryPropertyFlags RequiredTypes, VkDeviceMemory* OutMemory);
int MapDataToBuffer (VkDevice GraphicsDevice, VkDeviceMemory Memory, VkDeviceSize Offset, VkDeviceSize Size, void* DataSource);

int CreateImage (VkDevice GraphicsDevice, uint32_t GraphicsQueueFamilyIndex, VkExtent3D Extent, uint32_t ArrayLayers, VkFormat Format, VkImageLayout InitialLayout, VkSharingMode SharingMode, VkImage* OutImage);
int AllocateBindImageMemory (VkDevice GraphicsDevice, VkImage* Images, uint32_t ImageCount, VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, VkMemoryPropertyFlags RequiredTypes, VkDeviceMemory* OutMemory);

int ChangeImageLayout ();

int CopyBufferToBuffer (VkDevice GraphicsDevice, VkCommandPool CommandPool, VkQueue GraphicsQueue, VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size);
int CopyBufferToImage ();

int CreateShader (char* FilePath, VkDevice GraphicsDevice, VkShaderStageFlagBits ShaderStage, VkShaderModule* ShaderModule, VkPipelineShaderStageCreateInfo* ShaderStageCreateInfo);

int GetMemoryTypeIndex (VkMemoryRequirements MemoryRequirements, VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, VkMemoryPropertyFlags RequiredMemoryTypes, uint32_t* MemoryTypeIndex);
int SubmitOneTimeCmd (VkQueue GraphicsQueue, VkCommandBuffer CommandBuffer);

void DestroyBufferAndBufferMemory (VkDevice GraphicsDevice, VkBuffer Buffer, VkDeviceMemory BufferMemory);