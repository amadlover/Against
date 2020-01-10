#pragma once

#include <stdint.h>

#include "SplashScreenGraphics.h"

typedef struct _SplashScreen
{
	_SplashScreenGraphics SplashScreenGraphics;
} SplashScreen;

int InitSplashScreen ();
int DrawSplashScreen (uint64_t ElapsedTime);
void DestroySplashScreen ();