#pragma once

#include <Windows.h>

int ProcessMouseLeftClick ();
int ProcessMouseMiddleClick ();
int ProcessMouseRightClick ();

int ProcessMouseMovement (WPARAM wParam, LPARAM lParam);
int ProcessKeyboardInput (WPARAM wParam, LPARAM lParam);

int GameInit (HINSTANCE HInstance, HWND HWnd);
int GameMainLoop ();
void GameShutdown ();