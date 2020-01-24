#pragma once

#include <vulkan/vulkan.h>

typedef struct
{
	char Name[256];

	VkDeviceSize Width;
	VkDeviceSize Height;
	VkDeviceSize BPP;

	uint8_t* Pixels;

	VkDeviceSize Size;
	VkDeviceSize Offset;

	VkImage* Image;
	uint32_t LayerIndex;
	VkImageView* ImageView;
} Image;

int ImportImages (const char* FilePath, Image** Images, uint32_t* ImageCount);