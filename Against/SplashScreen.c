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
	SplashScreenObj = (SplashScreen*)MyCalloc (1, sizeof (SplashScreen));

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
			MyFree (SplashScreenObj->Actors);
		}

		if (SplashScreenObj->Assets)
		{
			for (uint32_t a = 0; a < SplashScreenObj->AssetCount; a++)
			{
				Asset* CurrentAsset = SplashScreenObj->Assets + a;

				if (CurrentAsset->GraphicsPrimitives)
				{
					for (uint32_t i = 0; i < CurrentAsset->GraphicsPrimitiveCount; i++)
					{
						GraphicsPrimitive* CurrentGP = CurrentAsset->GraphicsPrimitives + i;

						if (CurrentGP->Indices)
						{
							MyFree (CurrentGP->Indices);
						}

						if (CurrentGP->Positions)
						{
							MyFree (CurrentGP->Positions);
						}

						if (CurrentGP->UV0s)
						{
							MyFree (CurrentGP->UV0s);
						}

						if (CurrentGP->Material.BaseColorTexture.Image.Pixels)
						{
							stbi_image_free (CurrentGP->Material.BaseColorTexture.Image.Pixels);
						}

						MyFree (CurrentGP);
					}
				}

				if ((SplashScreenObj->Assets + a)->PhysicsPrimitives)
				{
					for (uint32_t i = 0; i < CurrentAsset->PhysicsPrimitiveCount; i++)
					{
						PhysicsPrimitive* CurrentPP = CurrentAsset->PhysicsPrimitives + i;

						if (CurrentPP->Indices)
						{
							MyFree ((CurrentAsset->PhysicsPrimitives + i)->Indices);
						}

						if (CurrentPP->Positions)
						{
							MyFree (CurrentPP->Positions);
						}

						MyFree (CurrentPP);
					}
				}
			}

			MyFree (SplashScreenObj->Assets);
		}

		MyFree (SplashScreenObj);
	}

	DestroySplashScreenGraphics ();
}