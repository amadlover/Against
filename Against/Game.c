#include "Game.h"
#include "Graphics.h"
#include "GUI.h"
#include "SplashScreen.h"
#include "MainMenu.h"
#include "MainGame.h"
#include "Sound.hpp"

#include <stdio.h>

enum _SceneState
{
	SplashScreen,
	MainMenu,
	MainGame,
};

enum _SceneState SceneState = SplashScreen;

enum _OverlayMenuState
{
	NoMenu,
	QuitMenu,
	PauseMenu,
};

enum _OverlayMenuState OverlayMenuState = NoMenu;

ULONGLONG StartupTickCount;
ULONGLONG SplashScreenThresholdTimeMS = 3000;

int GameInit (HINSTANCE HInstance, HWND HWnd)
{
	int Result = GraphicsInit (HInstance, HWnd);

	if (Result != 0)
	{
		return Result;
	}

	StartupTickCount = GetTickCount64 ();

	return 0;
}

int ProcessMouseLeftClick ()
{
	switch (OverlayMenuState)
	{
	case QuitMenu:
		break;

	case PauseMenu:
		break;

	default:
		break;
	}

	switch (SceneState)
	{
	case SplashScreen:
		break;

	case MainMenu:
		break;

	case MainGame:
		break;

	default:
		break;
	}

	return 0;
}

int ProcessMouseMiddleClick ()
{
	return 0;
}

int ProcessMouseRightClick ()
{
	return 0;
}

int ProcessMouseMovement (WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int ProcessKeyboardInput (WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0x57)
	{
		OutputDebugString (L"W\n");
	}

	if (wParam == 0x41)
	{
		OutputDebugString (L"A\n");
	}

	if (wParam == 0x53)
	{
		OutputDebugString (L"S\n");
	}

	if (wParam == 0x44)
	{
		OutputDebugString (L"D\n");
	}

	if (wParam == VK_ESCAPE)
	{
		OutputDebugString (L"ESC\n");

		switch (SceneState)
		{
		case SplashScreen:
			SceneState = MainMenu;
			break;

		case MainMenu:
			OverlayMenuState = QuitMenu;
			break;

		case MainGame:
			OverlayMenuState = PauseMenu;
			break;

		default:
			break;
		}

		switch (OverlayMenuState)
		{
		case NoMenu:
			break;

		case QuitMenu:
			OverlayMenuState = NoMenu;
			break;

		case PauseMenu:
			OverlayMenuState = NoMenu;
			break;

		default:
			break;
		}

		PostQuitMessage (0);
	}

	return 0;
}

int IsLoadingScreenToBeShown ()
{
	ULONGLONG CurrentTickCount = GetTickCount64 ();
	ULONGLONG ElapsedTime = CurrentTickCount - StartupTickCount;

	return ElapsedTime < SplashScreenThresholdTimeMS;		
}

int GameMainLoop ()
{
	switch (SceneState)
	{
	case SplashScreen:
		if (IsLoadingScreenToBeShown () == 0) 
		{
			SceneState = MainMenu;
		}
		else
		{
			DrawSplashScreen ();
		}

		break;

	case MainMenu:
		DrawMainGame ();
		break;

	case MainGame:
		break;

	default:
		break;
	}

	switch (OverlayMenuState)
	{
	case NoMenu:
		break;
	case QuitMenu:
		DrawQuitMenu ();
		break;
	case PauseMenu:
		DrawPauseMenu ();
		break;
	default:
		break;
	}

	return 0;
}

void GameShutdown ()
{
	OutputDebugString (L"GameShutdown\n");

	GraphicsShutdown ();
}