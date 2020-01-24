#include "splash_screen.h"
#include "graphics.h"
#include "error.h"
#include "asset.h"
#include "actor.h"
#include "import_assets.h"
#include "utils.h"
#include "graphics_utils.h"

#include "event.h"

#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_ONLY_TGA
#include <stb_image.h>

#include <Shlwapi.h>
#include <strsafe.h>

SplashScreen* SplashScreenObj;

int splash_screen_init ()
{
	SplashScreenObj = (SplashScreen*)my_calloc (1, sizeof (SplashScreen));

	if (SplashScreenObj == NULL)
	{
		return AGAINST_ERROR_SYSTEM_ALLOCATE_MEMORY;
	}

	char FilePath[MAX_PATH];
	get_full_file_path (FilePath, "\\UIElements\\SplashScreen\\SplashScreen.gltf");

	int Result = import_images (
		FilePath,
		&SplashScreenObj->Images,
		&SplashScreenObj->ImageCount
	);

	if (Result != 0)
	{
		return Result;
	}

	Result = ImportAssets (
		FilePath,
		&SplashScreenObj->Assets,
		&SplashScreenObj->AssetCount,
		SplashScreenObj->Images
	);

	if (Result != 0)
	{
		return Result;
	}

	Result = InitSplashScreenGraphics (
		SplashScreenObj->Assets,
		SplashScreenObj->AssetCount,
		SplashScreenObj->Images,
		SplashScreenObj->ImageCount
	);

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

int splash_screen_process_keyboard_input (WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_ESCAPE:
		go_to_scene_fp (e_scene_type_main_menu);
		break;

	default:
		break;
	}

	return 0;
}

int splash_screen_main_loop (uint64_t ElapsedTime)
{
	int Result = DrawSplashScreenGraphics ();

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

void DestroySplashScreenAssets ()
{
	OutputDebugString (L"DestroySplashScreenAssets\n");

	if (SplashScreenObj)
	{
		if (SplashScreenObj->Actors)
		{
			my_free (SplashScreenObj->Actors);
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
							my_free (CurrentGP->Indices);
						}

						if (CurrentGP->Positions)
						{
							my_free (CurrentGP->Positions);
						}

						if (CurrentGP->UV0s)
						{
							my_free (CurrentGP->UV0s);
						}

						if (CurrentGP->Normals)
						{
							my_free (CurrentGP->Normals);
						}

						my_free (CurrentGP);
					}
				}

				if ((SplashScreenObj->Assets + a)->PhysicsPrimitives)
				{
					for (uint32_t i = 0; i < CurrentAsset->PhysicsPrimitiveCount; i++)
					{
						PhysicsPrimitive* CurrentPP = CurrentAsset->PhysicsPrimitives + i;

						if (CurrentPP->Indices)
						{
							my_free (CurrentPP->Indices);
						}

						if (CurrentPP->Positions)
						{
							my_free (CurrentPP->Positions);
						}

						my_free (CurrentPP);
					}
				}
			}

			my_free (SplashScreenObj->Assets);
		}

		if (SplashScreenObj->Images)
		{
			my_free (SplashScreenObj->Images);
		}

		my_free (SplashScreenObj);
	}
}

void splash_screen_exit ()
{
	DestroySplashScreenGraphics ();
	DestroySplashScreenAssets ();
}