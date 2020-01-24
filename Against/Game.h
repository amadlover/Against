#pragma once

#include <Windows.h>

int process_left_mouse_click ();
int process_middle_mouse_click ();
int process_right_mouse_click ();

int process_mouse_movement (WPARAM wParam, LPARAM lParam);
int process_keyboard_input (WPARAM wParam, LPARAM lParam);

int GameInit (HINSTANCE HInstance, HWND HWnd);
int GameMainLoop ();
void GameShutdown ();