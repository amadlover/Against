#pragma once

#include <vulkan/vulkan.h>

int CreateBufferAndBufferMemory (VkDevice GraphicsDevice, VkDeviceSize Size, VkBufferUsageFlags Usage, VkSharingMode SharingMode, uint32_t GraphicsQueueFamilyIndex, VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, VkMemoryPropertyFlags RequiredTypes, VkBuffer* OutBuffer, VkDeviceMemory* OutBufferMemory);
int CopyDataToBuffer (VkDevice GraphicsDevice, VkDeviceMemory Memory, VkDeviceSize Offset, VkDeviceSize Size, void* DataSource);
int CreateImageAndImageMemory ();
int ChangeImageLayout ();
int CopyBufferToBuffer (VkDevice GraphicsDevice, VkCommandPool CommandPool, VkQueue GraphicsQueue, VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size);
int CopyBufferToImage ();

int CreateShader (char* FilePath, VkDevice GraphicsDevice, VkShaderStageFlagBits ShaderStage, VkShaderModule* ShaderModule, VkPipelineShaderStageCreateInfo* ShaderStageCreateInfo);

int GetMemoryTypeIndex (VkMemoryRequirements MemoryRequirements, VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, VkMemoryPropertyFlags RequiredMemoryTypes, uint32_t* MemoryTypeIndex);
int SubmitOneTimeCmd (VkQueue GraphicsQueue, VkCommandBuffer CommandBuffer);

void DestroyBufferAndBufferMemory (VkDevice GraphicsDevice, VkBuffer Buffer, VkDeviceMemory BufferMemory);