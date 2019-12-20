#include "Game.h"
#include "Physics.h"
#include "Graphics.h"
#include "GUI.h"
#include "SplashScreen.h"
#include "MainMenu.h"
#include "MainGame.h"

#include <stdio.h>
#include <stdbool.h>

#include <Windowsx.h>

enum _CurrentScene
{
	SplashScreen,
	MainMenu,
	MainGame,
};

enum _CurrentScene CurrentScene = SplashScreen;

enum _OverlayMenuState
{
	NoMenu,
	QuitMenu,
	PauseMenu,
};

enum _OverlayMenuState OverlayMenuState = NoMenu;

enum _SceneState
{
	Exited,
	Inited
};

enum _SceneState SplashScreenSceneState = Exited;
enum _SceneState MainMenuSceneState = Exited;
enum _SceneState MainGameSceneState = Exited;

uint64_t StartupTickCount;
uint64_t CurrentTickCount;
uint64_t ElapsedTime;
uint64_t SplashScreenThresholdTimeMS = 3000;

uint32_t LastMouseX;
uint32_t LastMouseY;

int ProcessMouseLeftClick ()
{
	switch (CurrentScene)
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
	switch (CurrentScene)
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

int ProcessMouseRightClick ()
{
	switch (CurrentScene)
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

int ProcessMouseMovement (WPARAM wParam, LPARAM lParam)
{
	uint32_t CurrentX = GET_X_LPARAM (lParam);
	uint32_t CurrentY = GET_Y_LPARAM (lParam);

	switch (CurrentScene)
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

	LastMouseX = CurrentX;
	LastMouseY = CurrentY;

	return 0;
}

int ProcessKeyboardInput (WPARAM wParam, LPARAM lParam)
{
	switch (CurrentScene)
	{
	case SplashScreen:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroySplashScreenGraphics ();
			SplashScreenSceneState = Exited;

			CurrentScene = MainMenu;
			break;
		default:
			break;
		}
		break;

	case MainMenu:
		break;

	case MainGame:
		break;

	default:
		break;
	}

	/*switch (wParam)
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
	}*/

	return 0;
}

int GameInit (HINSTANCE HInstance, HWND HWnd)
{
	OutputDebugString (L"GameInit\n");

	int Result = PhysicsInit ();

	if (Result != 0)
	{
		return Result;
	}

	Result = GraphicsInit (HInstance, HWnd);

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

	switch (CurrentScene)
	{
	case SplashScreen:
		if (ElapsedTime > SplashScreenThresholdTimeMS)
		{
			OutputDebugString (L"Switching to Main Menu\n");
			CurrentScene = MainMenu;

			DestroySplashScreenGraphics ();

			SplashScreenSceneState = Exited;
		}
		else
		{
			if (SplashScreenSceneState == Exited)
			{
				Result = InitSplashScreenGraphics ();

				if (Result != 0)
				{
					return Result;
				}

				SplashScreenSceneState = Inited;
			}
			else if (SplashScreenSceneState == Inited)
			{
				Result = DrawSplashScreen (ElapsedTime);

				if (Result != 0)
				{
					return Result;
				}
			}
		}

		break;

	case MainMenu:
		if (MainMenuSceneState == Exited)
		{
			Result = InitMainMenuGraphics ();

			if (Result != 0)
			{
				return Result;
			}

			MainMenuSceneState = Inited;
		}
		else if (MainMenuSceneState == Inited)
		{
			Result = DrawMainMenu ();

			if (Result != 0)
			{
				return Result;
			}
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

	return 0;
}

void GameShutdown ()
{
	OutputDebugString (L"GameShutdown\n");

	if (SplashScreenSceneState == Inited)
	{
		DestroySplashScreenGraphics ();
		SplashScreenSceneState = Exited;
	}

	if (MainMenuSceneState == Inited)
	{
		DestroyMainMenuGraphics ();
		MainMenuSceneState = Exited;
	}

	if (MainGameSceneState == Inited)
	{
		DestroyMainGameGraphics ();
		MainGameSceneState = Exited;
	}

	GraphicsShutdown ();

	PhysicsShutdown ();
}