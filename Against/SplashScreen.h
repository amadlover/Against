#pragma once

typedef struct _SplashScreen
{
	int x;
} SplashScreenObj;

int SetupSplashScreen ();
int DrawSplashScreen ();
void DestroySplashScreen ();
