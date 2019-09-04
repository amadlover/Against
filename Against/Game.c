#include "Game.h"
#include "Graphics.h"
#include "GUI.h"
#include "SplashScreen.h"
#include "MainMenu.h"
#include "MainGame.h"

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

uint64_t StartupTickCount;
uint64_t CurrentTickCount;
uint64_t ElapsedTime;
uint64_t SplashScreenThresholdTimeMS = 3000;

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

int GameMainLoop ()
{
	CurrentTickCount = GetTickCount64 ();
	ElapsedTime = CurrentTickCount - StartupTickCount;

	int Result = 0;

	switch (SceneState)
	{
	case SplashScreen:
		if (ElapsedTime > SplashScreenThresholdTimeMS)
		{
			OutputDebugString (L"Switching to Main Menu\n");
			SceneState = MainMenu;
		}
		else
		{
			Result = DrawSplashScreen (ElapsedTime);

			if (Result != 0)
			{
				return Result;
			}
		}

		break;

	case MainMenu:
		Result = DrawMainMenu ();

		if (Result != 0)
		{
			return Result;
		}

		break;

	case MainGame:
		Result = DrawMainGame ();

		if (Result != 0)
		{
			return Result;
		}

		break;

	default:
		break;
	}

	switch (OverlayMenuState)
	{
	case NoMenu:
		break;
	case QuitMenu:
		Result = DrawQuitMenu ();

		if (Result != 0)
		{
			return Result;
		}

		break;
	case PauseMenu:
		Result = DrawPauseMenu ();

		if (Result != 0)
		{
			return Result;
		}

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