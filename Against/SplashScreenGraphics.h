#pragma once

#include "Graphics.h"
#include "Asset.h"
#include "Actor.h"

typedef struct __SplashScreenGraphics
{
	int DUMMY;
}_SplashScreenGraphics;

int InitSplashScreenGraphics (Asset* Assets, uint32_t AssetCount);
int DrawSplashScreenGraphics ();
void DestroySplashScreenGraphics ();