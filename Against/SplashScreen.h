#pragma once

#include <stdint.h>

#include "SplashScreenGraphics.h"
#include "Asset.h"
#include "Actor.h"
#include "Image.h"

typedef struct
{
	_SplashScreenGraphics SplashScreenGraphics;

	Asset* Assets;
	uint32_t AssetCount;

	Actor* Actors;
	uint32_t ActorCount;

	Image* Images;
	uint32_t ImageCount;
} SplashScreen;

int InitSplashScreen ();
int DrawSplashScreen (uint64_t ElapsedTime);
void DestroySplashScreen ();