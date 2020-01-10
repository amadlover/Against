#pragma once

#include <vulkan/vulkan.h>

int CreateBufferAndBufferMemory (VkDevice GraphicsDevice, VkDeviceSize Size, VkBufferUsageFlags Usage, VkSharingMode SharingMode, uint32_t GraphicsQueueFamilyIndex, VkMemoryPropertyFlags RequiredTypes, VkBuffer* OutBuffer, VkDeviceMemory* OutBufferMemory);
int CreateShader (char* FilePath, VkShaderStageFlagBits ShaderStage, VkShaderModule* ShaderModule, VkPipelineShaderStageCreateInfo* ShaderStageCreateInfo);
int CreateTextureImageOnDevice (char* FilePath, VkImage* TextureImage, VkDeviceMemory* TextureImageMemory);

int GetMemoryTypeIndex (VkMemoryRequirements MemoryRequirements, VkPhysicalDeviceMemoryProperties PhysicalDeviceMemoryProperties, VkMemoryPropertyFlags RequiredMemoryTypes, uint32_t* MemoryTypeIndex);