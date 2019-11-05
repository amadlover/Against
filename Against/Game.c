#include "Game.h"
#include "Graphics.h"
#include "GUI.h"
#include "SplashScreen.h"
#include "MainMenu.h"
#include "MainGame.h"

#include <stdio.h>

#include <Windowsx.h>

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

uint32_t LastMouseX;
uint32_t LastMouseY;

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
	uint32_t CurrentX = GET_X_LPARAM (lParam);
	uint32_t CurrentY = GET_Y_LPARAM (lParam);

	switch (OverlayMenuState)
	{
	case NoMenu:
		switch (SceneState)
		{
		case SplashScreen:
			break;

		case MainMenu:
			MainMenuProcessMouseMovement (CurrentX, CurrentY, CurrentX - LastMouseX, CurrentY - LastMouseY);
			break;

		case MainGame:
			break;

		default:
			break;
		}

	case QuitMenu:
		break;

	case PauseMenu:
		break;

	default:
		break;
	}

	LastMouseX = CurrentX;
	LastMouseY = CurrentY;

	return 0;
}

int ProcessKeyboardInput (WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case 0x57:
		OutputDebugString (L"W\n");
		break;

	case 0x41:
		OutputDebugString (L"A\n");
		break;

	case 0x53:
		OutputDebugString (L"S\n");
		break;

	case 0x44:
		OutputDebugString (L"D\n");
		break;

	case VK_ESCAPE:
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

	default:
		break;
	}

	return 0;
}

int GameInit (HINSTANCE HInstance, HWND HWnd)
{
	OutputDebugString (L"GameInit\n");

	int Result = GraphicsInit (HInstance, HWnd);

	if (Result != 0)
	{
		return Result;
	}

	StartupTickCount = GetTickCount64 ();

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
	
	OutputDebugString (L"Finished GameShutdown\n");
}