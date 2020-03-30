#include "game.h"
#include "common_graphics.h"
#include "enums.h"
#include "utils.h"

#include <stdio.h>
#include <stdbool.h>

#include <Windowsx.h>

e_scene_state splash_screen_state = e_scene_state_exited;
e_scene_state main_menu_state = e_scene_state_exited;

int (*current_scene_init)();
int (*current_scene_process_keyboard_input)(WPARAM, LPARAM);
int (*current_scene_main_loop)();
void (*current_scene_exit)();

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

int game_init (HINSTANCE hInstance, HWND hWnd)
{
	OutputDebugString (L"game_init\n");

	CHECK_AGAINST_RESULT (common_graphics_init (hInstance, hWnd));
	CHECK_AGAINST_RESULT (set_current_scene (e_scene_type_test));

	splash_screen_state = e_scene_state_inited;

	return 0;
}

int set_current_scene (e_scene_type scene_type)
{
	if (current_scene_exit != NULL)
	{
		current_scene_exit ();
	}

	switch (scene_type)
	{
	default:
		break;
	}

	if (current_scene_init != NULL)
	{
		current_scene_init ();
	}

	return 0;
}

int game_main_loop ()
{
	if (current_scene_main_loop != NULL)
	{
		CHECK_AGAINST_RESULT (current_scene_main_loop ());
	}

	return 0;
}

void game_exit ()
{
	OutputDebugString (L"game_exit\n");

	if (current_scene_exit != NULL)
	{
		current_scene_exit ();
	}

	common_graphics_exit ();
}