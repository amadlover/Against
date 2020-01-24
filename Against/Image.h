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

	VkImage* image;
	uint32_t LayerIndex;
	VkImageView* ImageView;
} image;

int import_images (const char* FilePath, image** Images, uint32_t* ImageCount);