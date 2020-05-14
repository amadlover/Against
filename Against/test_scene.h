#pragma once

#include "error.h"

#include <Windows.h>

AGAINST_RESULT test_scene_init ();
AGAINST_RESULT test_scene_process_keyboard_input (WPARAM w_param, LPARAM l_param);
AGAINST_RESULT test_scene_main_loop ();
void test_scene_exit ();