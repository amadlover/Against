#include "game.h"
#include "physics.h"
#include "graphics.h"
#include "gui.h"
#include "splash_screen.h"
#include "main_menu.h"
#include "main_game.h"
#include "event.h"
#include "enums.h"

#include <stdio.h>
#include <stdbool.h>

#include <Windowsx.h>

e_scene_type current_scene = e_scene_type_splash_screen;;

e_scene_state splash_screen_state = e_scene_state_exited;
e_scene_state main_menu_state = e_scene_state_exited;

int (*current_scene_process_keyboard_input)(WPARAM, LPARAM);
int (*current_scene_draw)();

int process_left_mouse_click ()
{
	return 0;
}

int process_middle_mouse_click ()
{
	return 0;
}

int process_right_mouse_click ()
{
	return 0;
}

int process_mouse_movement (WPARAM wParam, LPARAM lParam)
{
	return 0;
}

int process_keyboard_input (WPARAM wParam, LPARAM lParam)
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
		main_menu_state = e_scene_state_exited;

		current_scene = e_scene_type_splash_screen;
		current_scene_process_keyboard_input = splash_screen_process_keyboard_input;
		current_scene_draw = splash_screen_draw;

		splash_screen_init ();
		splash_screen_state = e_scene_state_inited;
		break;

	case e_scene_type_main_menu:
		DestroySplashScreen ();
		splash_screen_state = e_scene_state_exited;

		current_scene = e_scene_type_main_menu;
		current_scene_process_keyboard_input = main_menu_process_keyboard_input;
		current_scene_draw = main_menu_draw;

		InitMainMenuGraphics ();
		main_menu_state = e_scene_state_inited;
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

	current_scene_process_keyboard_input = splash_screen_process_keyboard_input;
	current_scene_draw = splash_screen_draw;
	go_to_scene_fp = go_to_scene;

	splash_screen_init ();

	return 0;
}

int GameMainLoop ()
{
	int Result = 0;

	Result = current_scene_draw (0);

	if (Result != 0)
	{
		return Result;
	}

	return 0;
}

void GameShutdown ()
{
	OutputDebugString (L"GameShutdown\n");

	if (splash_screen_state == e_scene_state_inited)
	{
		DestroySplashScreen ();
		splash_screen_state = e_scene_state_exited;
	}

	if (main_menu_state == e_scene_state_inited)
	{
		DestroyMainMenuGraphics ();
		main_menu_state = e_scene_state_exited;
	}

	GraphicsShutdown ();

	PhysicsShutdown ();
}