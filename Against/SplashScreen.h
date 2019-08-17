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

int SetupSplashScreen ();
int SetupMainMenu ();
int SetupMainGame ();

int DrawSplashScreen ();
int DrawMainMenu ();
int DrawMainGame ();
