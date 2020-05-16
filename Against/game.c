#include "game.h"
#include "common_graphics.h"
#include "enums.h"
#include "utils.h"
#include "test_scene.h"
#include "error.h"

#include <stdio.h>
#include <stdbool.h>

#include <Windowsx.h>

AGAINST_RESULT (*current_scene_init)();
AGAINST_RESULT (*current_scene_process_keyboard_input)(WPARAM, LPARAM);
AGAINST_RESULT (*current_scene_main_loop)();
void (*current_scene_shutdown)();

AGAINST_RESULT game_process_left_mouse_click ()
{
	return AGAINST_SUCCESS;
}

AGAINST_RESULT game_process_middle_mouse_click ()
{
	return AGAINST_SUCCESS;
}

AGAINST_RESULT game_process_right_mouse_click ()
{
	return AGAINST_SUCCESS;
}

AGAINST_RESULT game_process_mouse_movement (WPARAM wParam, LPARAM lParam)
{
	return AGAINST_SUCCESS;
}

AGAINST_RESULT game_process_keyboard_input (WPARAM wParam, LPARAM lParam)
{
	if (current_scene_process_keyboard_input != NULL)
	{
		AGAINST_RESULT result;
		CHECK_AGAINST_RESULT (current_scene_process_keyboard_input (wParam, lParam), result);
	}

	return AGAINST_SUCCESS;
}

AGAINST_RESULT game_init (HINSTANCE hInstance, HWND hWnd)
{
	OutputDebugString (L"game_init\n");

	AGAINST_RESULT result;

	CHECK_AGAINST_RESULT (common_graphics_init (hInstance, hWnd), result);
	CHECK_AGAINST_RESULT (game_set_current_scene (e_scene_type_test), result);

	return AGAINST_SUCCESS;
}

AGAINST_RESULT game_set_current_scene (e_scene_type scene_type)
{
	OutputDebugString (L"game_set_current_scene\n");
	
	if (current_scene_shutdown != NULL)
	{
		current_scene_shutdown ();
	}

	switch (scene_type)
	{
	case e_scene_type_test:
		current_scene_init = test_scene_init;
		current_scene_process_keyboard_input = test_scene_process_keyboard_input;
		current_scene_main_loop = test_scene_main_loop;
		current_scene_shutdown = test_scene_shutdown;
	default:
		break;
	}

	if (current_scene_init != NULL)
	{
		AGAINST_RESULT result;
		CHECK_AGAINST_RESULT (current_scene_init (), result);
	}

	return AGAINST_SUCCESS;
}

AGAINST_RESULT game_main_loop ()
{
	if (current_scene_main_loop != NULL)
	{
		AGAINST_RESULT result;
		CHECK_AGAINST_RESULT (current_scene_main_loop (), result);
	}

	return AGAINST_SUCCESS;
}

void game_shutdown ()
{
	OutputDebugString (L"game_shutdown\n");

	if (current_scene_shutdown != NULL)
	{
		current_scene_shutdown ();
	}

	common_graphics_shutdown ();
}