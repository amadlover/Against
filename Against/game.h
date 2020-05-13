#pragma once

#include "error.h"
#include "enums.h"
#include <Windows.h>

AGAINST_RESULT game_process_left_mouse_click ();
AGAINST_RESULT game_process_middle_mouse_click ();
AGAINST_RESULT game_process_right_mouse_click ();

AGAINST_RESULT game_process_mouse_movement (WPARAM wParam, LPARAM lParam);
AGAINST_RESULT game_process_keyboard_input (WPARAM wParam, LPARAM lParam);

AGAINST_RESULT game_init (HINSTANCE HInstance, HWND HWnd);
AGAINST_RESULT game_set_current_scene (e_scene_type scene_type);
AGAINST_RESULT game_main_loop ();
void game_exit ();