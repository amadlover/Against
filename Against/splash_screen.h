#pragma once

#include <stdint.h>

#include "splash_screen_graphics.h"
#include "Asset.h"
#include "Actor.h"
#include "Image.h"

typedef struct
{
	_SplashScreenGraphics SplashScreenGraphics;

	Asset* Assets;
	uint32_t AssetCount;

	actor* Actors;
	uint32_t ActorCount;

	image* Images;
	uint32_t ImageCount;
} SplashScreen;

int splash_screen_init ();
int splash_screen_process_keyboard_input (WPARAM wParam, LPARAM lParam);
int splash_screen_main_loop (uint64_t ElapsedTime);
void splash_screen_exit ();