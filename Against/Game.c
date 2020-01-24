#include "game.h"
#include "physics.h"
#include "graphics.h"
#include "gui.h"
#include "splash_screen.h"
#include "main_menu.h"
#include "main_game.h"
#include "event.h"

#include <stdio.h>
#include <stdbool.h>

#include <Windowsx.h>

enum _CurrentScene
{
	SplashScreenScene,
	MainMenuScene,
	MainGameScene,
};

enum _CurrentScene CurrentScene = SplashScreenScene;

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

int (*current_scene_process_keyboard_input)(WPARAM, LPARAM);
int (*current_scene_draw)();

int ProcessMouseLeftClick ()
{
	switch (CurrentScene)
	{
	case SplashScreenScene:
		break;

	case MainMenuScene:
		break;

	case MainGameScene:
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
	case SplashScreenScene:
		break;

	case MainMenuScene:
		break;

	case MainGameScene:
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
	case SplashScreenScene:
		break;

	case MainMenuScene:
		break;

	case MainGameScene:
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
	case SplashScreenScene:
		break;

	case MainMenuScene:
		break;

	case MainGameScene:
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
	current_scene_process_keyboard_input (wParam, lParam);
	return 0;
}

void go_to_scene (e_scene_type scene_type)
{
	switch (scene_type)
	{
	case e_scene_type_splash_screen:
		DestroyMainMenuGraphics ();
		MainMenuSceneState = Exited;
		CurrentScene = SplashScreenScene;
		
		current_scene_process_keyboard_input = splash_screen_process_keyboard_input;
		current_scene_draw = splash_screen_draw;

		splash_screen_init ();
		SplashScreenSceneState = Inited;
		break;

	case e_scene_type_main_menu:
		DestroySplashScreen ();
		SplashScreenSceneState = Exited;
		
		CurrentScene = MainMenuScene;
		current_scene_process_keyboard_input = main_menu_process_keyboard_input;
		current_scene_draw = main_menu_draw;

		InitMainMenuGraphics ();
		MainMenuSceneState = Inited;
		break;

	default:
		break;
	}
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

	current_scene_process_keyboard_input = splash_screen_process_keyboard_input;
	current_scene_draw = splash_screen_draw;
	go_to_scene_fp = go_to_scene;

	splash_screen_init ();

	return 0;
}

int GameMainLoop ()
{
	CurrentTickCount = GetTickCount64 ();
	ElapsedTime = CurrentTickCount - StartupTickCount;

	int Result = 0;

	Result = current_scene_draw (ElapsedTime);

	if (Result != 0)
	{
		return Result;
	}
	
	return 0;
}

void GameShutdown ()
{
	OutputDebugString (L"GameShutdown\n");

	if (SplashScreenSceneState == Inited)
	{
		DestroySplashScreen ();
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