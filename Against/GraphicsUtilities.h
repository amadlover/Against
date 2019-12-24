#pragma once

#include <vulkan/vulkan.h>

int CreateUniformBuffer (VkBuffer* UniformBuffer, VkDeviceMemory* UniformBufferMemory, VkDeviceSize Size, VkBufferUsageFlags BufferUsage, VkMemoryPropertyFlags MemoryPropertyFlags);
int CreateShader (char* FilePath, VkShaderStageFlags ShaderStage, VkPipelineShaderStageCreateInfo* ShaderStageCreateInfo);
int CreateTextureOnDevice (char* FilePath, VkImage* TextureImage, VkDeviceMemory* TextureImageMemory);