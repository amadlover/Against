#include "game.h"
#include "common_graphics.h"
#include "enums.h"
#include "utils.h"
#include "test_scene.h"
#include "error.h"

#include <stdio.h>
#include <stdbool.h>

#include <Windowsx.h>

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
	if (current_scene_process_keyboard_input != NULL)
	{
		AGAINSTRESULT result;
		CHECK_AGAINST_RESULT (current_scene_process_keyboard_input (wParam, lParam), result);
	}

	return 0;
}

int game_init (HINSTANCE hInstance, HWND hWnd)
{
	OutputDebugString (L"game_init\n");

	AGAINSTRESULT result;

	//CHECK_AGAINST_RESULT (common_graphics_init (hInstance, hWnd), result);
	CHECK_AGAINST_RESULT (game_set_current_scene (e_scene_type_test), result);

	return 0;
}

int game_set_current_scene (e_scene_type scene_type)
{
	OutputDebugString (L"game_set_current_scene\n");
	
	if (current_scene_exit != NULL)
	{
		current_scene_exit ();
	}

	switch (scene_type)
	{
	case e_scene_type_test:
		current_scene_init = test_scene_init;
		current_scene_process_keyboard_input = test_scene_process_keyboard_input;
		current_scene_main_loop = test_scene_main_loop;
		current_scene_exit = test_scene_exit;
	default:
		break;
	}

	if (current_scene_init != NULL)
	{
		AGAINSTRESULT result;
		CHECK_AGAINST_RESULT (current_scene_init (), result);
	}

	return 0;
}

int game_main_loop ()
{
	if (current_scene_main_loop != NULL)
	{
		AGAINSTRESULT result;
		CHECK_AGAINST_RESULT (current_scene_main_loop (), result);
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

	//common_graphics_exit ();
}