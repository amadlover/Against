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

SplashScreen* SplashScreenObj;

int InitSplashScreen ()
{
	SplashScreenObj = (SplashScreen*)calloc (1, sizeof (SplashScreen));

	if (SplashScreenObj == NULL)
	{
		return AGAINST_ERROR_SYSTEM_ALLOCATE_MEMORY;
	}
	
	char FilePath[MAX_PATH];
	GetFullFilePath (FilePath, "\\UIElements\\SplashScreen\\SplashScreen.gltf");

	int Result = ImportAssets (FilePath, &SplashScreenObj->Assets, &SplashScreenObj->AssetCount);

	if (Result != 0)
	{
		return Result;
	}

	InitSplashScreenGraphics (SplashScreenObj->Assets, SplashScreenObj->AssetCount);

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
	if (SplashScreenObj )
	{
		if (SplashScreenObj->Actors)
		{
			free (SplashScreenObj->Actors);
		}

		if (SplashScreenObj->Assets)
		{
			free (SplashScreenObj->Assets);
		}

		free (SplashScreenObj);
	}

	DestroySplashScreenGraphics ();
}