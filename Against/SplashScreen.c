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
			for (uint32_t a = 0; a < SplashScreenObj->AssetCount; a++)
			{
				if ((SplashScreenObj->Assets + a)->GraphicsPrimitives)
				{
					for (uint32_t i = 0; i < (SplashScreenObj->Assets + a)->GraphicsPrimitiveCount; i++)
					{
						if (((SplashScreenObj->Assets + a)->GraphicsPrimitives + i)->Indices)
						{
							free ((SplashScreenObj->Assets->GraphicsPrimitives + i)->Indices);
						}

						if (((SplashScreenObj->Assets + a)->GraphicsPrimitives + i)->Positions)
						{
							free ((SplashScreenObj->Assets->GraphicsPrimitives + i)->Positions);
						}

						if (((SplashScreenObj->Assets + a)->GraphicsPrimitives + i)->UV0s)
						{
							free ((SplashScreenObj->Assets->GraphicsPrimitives + i)->UV0s);
						}

						free ((SplashScreenObj->Assets + a)->GraphicsPrimitives + i);
					}
				}

				if ((SplashScreenObj->Assets + a)->PhysicsPrimitives)
				{
					for (uint32_t i = 0; i < (SplashScreenObj->Assets + a)->PhysicsPrimitiveCount; i++)
					{
						if (((SplashScreenObj->Assets + a)->PhysicsPrimitives + i)->Indices)
						{
							free (((SplashScreenObj->Assets + a)->PhysicsPrimitives + i)->Indices);
						}

						if (((SplashScreenObj->Assets + a)->PhysicsPrimitives + i)->Positions)
						{
							free (((SplashScreenObj->Assets + a)->PhysicsPrimitives + i)->Positions);
						}

						free ((SplashScreenObj->Assets + a)->PhysicsPrimitives + i);
					}
				}
			}

			free (SplashScreenObj->Assets);
		}

		free (SplashScreenObj);
	}

	DestroySplashScreenGraphics ();
}