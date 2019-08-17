#pragma once

typedef struct _SplashScreen
{
	int x;
} SplashScreenObj;

typedef struct _MainMenu
{
	int x;
} MainMeuObj;

typedef struct _MainGame
{
	int x;
} MainGameObj;

void DrawSplashScreen ();
void DrawMainMenu ();
void DrawMainGame ();
