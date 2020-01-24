#pragma once

#include "Graphics.h"
#include "Asset.h"
#include "Actor.h"

typedef struct
{
	VkCommandPool CommandPool;

	VkBuffer GraphicsVBIBBuffer;
	VkDeviceMemory GraphicsVBIBBufferMemory;

	VkDeviceMemory GraphicsImageMemory;
	VkImage* Images;
	uint32_t ImageCount;

	VkImageView* ImageViews;
	uint32_t ImageViewCount;

} _SplashScreenGraphics;

int InitSplashScreenGraphics (Asset* Assets, uint32_t AssetCount, image* Images, uint32_t ImageCount);
int DrawSplashScreenGraphics ();
void DestroySplashScreenGraphics ();