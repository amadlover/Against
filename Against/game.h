#pragma once

#include <Windows.h>
#include "enums.h"

int game_process_left_mouse_click ();
int game_process_middle_mouse_click ();
int game_process_right_mouse_click ();

int game_process_mouse_movement (WPARAM wParam, LPARAM lParam);
int game_process_keyboard_input (WPARAM wParam, LPARAM lParam);

int game_init (HINSTANCE HInstance, HWND HWnd);
int game_set_current_scene (e_scene_type scene_type);
int game_main_loop ();
void game_exit ();