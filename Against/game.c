#include "game.h"
#include "graphics.h"
#include "splash_screen.h"
#include "main_menu.h"
#include "event.h"
#include "enums.h"

#include <stdio.h>
#include <stdbool.h>

#include <Windowsx.h>

e_scene_type current_scene = e_scene_type_splash_screen;;

e_scene_state splash_screen_state = e_scene_state_exited;
e_scene_state main_menu_state = e_scene_state_exited;

int (*current_scene_process_keyboard_input)(WPARAM, LPARAM);
int (*current_scene_main_loop)();

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

int go_to_scene (e_scene_type scene_type)
{
	int result = 0;
	switch (scene_type)
	{
	case e_scene_type_splash_screen:
		main_menu_exit ();
		main_menu_state = e_scene_state_exited;

		current_scene = e_scene_type_splash_screen;
		current_scene_process_keyboard_input = splash_screen_process_keyboard_input;
		current_scene_main_loop = splash_screen_main_loop;

		result = splash_screen_init ();
		if (result != 0)
		{
			return result;
		}
		splash_screen_state = e_scene_state_inited;
		break;

	case e_scene_type_main_menu:
		splash_screen_exit ();
		splash_screen_state = e_scene_state_exited;

		current_scene = e_scene_type_main_menu;
		current_scene_process_keyboard_input = main_menu_process_keyboard_input;
		current_scene_main_loop = main_menu_main_loop;

		result = main_menu_init ();
		if (result != 0)
		{
			return result;
		}
		main_menu_state = e_scene_state_inited;
		break;

	default:
		break;
	}

	return 0;
}

int game_init (HINSTANCE hInstance, HWND hWnd)
{
	OutputDebugString (L"game_init\n");

	int result = graphics_init (hInstance, hWnd);

	if (result != 0)
	{
		return result;
	}

	current_scene_process_keyboard_input = splash_screen_process_keyboard_input;
	current_scene_main_loop = splash_screen_main_loop;
	event_go_to_scene_fp = go_to_scene;

	result = splash_screen_init ();

	if (result != 0) 
	{
		return result;
	}

	return 0;
}

int game_main_loop ()
{
	int result = 0;

	result = current_scene_main_loop (0);

	if (result != 0)
	{
		return result;
	}

	return 0;
}

void game_exit ()
{
	OutputDebugString (L"game_exit\n");

	if (splash_screen_state == e_scene_state_inited)
	{
		splash_screen_exit ();
		splash_screen_state = e_scene_state_exited;
	}

	if (main_menu_state == e_scene_state_inited)
	{
		main_menu_exit ();
		main_menu_state = e_scene_state_exited;
	}

	graphics_exit ();
}