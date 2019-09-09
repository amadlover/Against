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

LRESULT CALLBACK WindowProc (HWND WindowHandle, UINT Msg, WPARAM wParam, LPARAM lParam);