#pragma once

#include <Windows.h>

int test_scene_init (HINSTANCE h_instnace, HWND h_wnd);
int test_scene_process_keyboard_input (WPARAM w_param, LPARAM l_param);
int test_scene_main_loop ();
void test_scene_exit ();