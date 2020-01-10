#pragma once

#include <stdint.h>

#include "SplashScreenGraphics.h"
#include "Asset.h"
#include "Actor.h"

typedef struct _SplashScreen
{
	_SplashScreenGraphics SplashScreenGraphics;

	Asset* Assets;
	uint32_t AssetCount;

	Actor* Actors;
	uint32_t ActorCount;
} SplashScreen;

int InitSplashScreen ();
int DrawSplashScreen (uint64_t ElapsedTime);
void DestroySplashScreen ();