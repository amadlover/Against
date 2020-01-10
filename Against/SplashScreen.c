#include "SplashScreen.h"
#include "Graphics.h"
#include "Error.h"
#include "Asset.h"
#include "Actor.h"
#include "ImportAssets.h"
#include "Utility.h"
#include "GraphicsUtilities.h"

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_ONLY_TGA
#include <stb_image.h>

#include <Shlwapi.h>
#include <strsafe.h>


int InitSplashScreen ()
{
	int Result = InitSplashScreenGraphics ();

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

int DrawSplashScreen (uint64_t ElapsedTime)
{
	int Result = DrawSplashScreenGraphics ();
	
	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

void DestroySplashScreen ()
{
	DestroySplashScreenGraphics ();
}