#pragma once

#include <Windows.h>

int splash_screen_init ();
int splash_screen_process_keyboard_input (WPARAM wParam, LPARAM lParam);
int splash_screen_main_loop ();
int splash_screen_exit ();
