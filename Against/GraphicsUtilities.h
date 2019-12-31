#pragma once

#include <vulkan/vulkan.h>

int CreateUniformBuffer (VkBuffer* UniformBuffer, VkDeviceMemory* UniformBufferMemory, VkDeviceSize Size, VkBufferUsageFlags BufferUsage, VkMemoryPropertyFlags MemoryPropertyFlags);
int CreateShader (char* FilePath, VkShaderStageFlagBits ShaderStage, VkPipelineShaderStageCreateInfo* ShaderStageCreateInfo);
int CreateTextureImageOnDevice (char* FilePath, VkImage* TextureImage, VkDeviceMemory* TextureImageMemory);