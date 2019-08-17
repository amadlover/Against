#include <Windows.h>

#include <stdio.h>

#include "Game.h"

LRESULT CALLBACK WindowProc (HWND HWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch (Msg)
	{
	case WM_DESTROY:
		PostQuitMessage (0);
		return 0;

	case WM_KEYDOWN:
		ProcessKeyboardInput (wParam, lParam);

		break;

	case WM_LBUTTONDOWN:
		ProcessMouseLeftClick ();

		break;

	case WM_MBUTTONDOWN:
		ProcessMouseMiddleClick ();
		
		break;

	case WM_RBUTTONDOWN:
		ProcessMouseRightClick ();

		break;

	case WM_MOUSEMOVE:
		ProcessMouseMovement (wParam, lParam);

		break;

	default:
		break;
	}

	return DefWindowProc (HWnd, Msg, wParam, lParam);
}

int WINAPI wWinMain (_In_ HINSTANCE HInstance, _In_opt_ HINSTANCE PreviousHInstance, _In_ PWSTR CmdLine, _In_ int CmdShow)
{
	WNDCLASS WC = { 0 };

	WC.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WC.lpfnWndProc = WindowProc;
	WC.hInstance = HInstance;
	WC.lpszClassName = L"Against";

	if (!RegisterClass (&WC))
	{
		return 0;
	}

	HWND HWnd = CreateWindow (L"Against", L"Against", WS_OVERLAPPEDWINDOW, 0, 0, 800, 600, NULL, NULL, HInstance, NULL);

	if (!HWnd)
	{
		return 0;
	}

	ShowWindow (HWnd, CmdShow);
	UpdateWindow (HWnd);

	int Result = GameInit (HInstance, HWnd);

	if (Result != 0)
	{
		wchar_t Buff[32];
		swprintf_s (Buff, 32, L"Init Error %d\n", Result);
		OutputDebugString (Buff);
		GameShutdown ();

		return Result;
	}

	MSG Msg;
	ZeroMemory (&Msg, sizeof (Msg));

	while (TRUE)
	{
		while (PeekMessage (&Msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage (&Msg);
			DispatchMessage (&Msg);
		}

		if (Msg.message == WM_QUIT)
		{
			break;
		}

		Result = GameMainLoop ();

		if (Result != 0)
		{
			wchar_t Buff[32];
			swprintf_s (Buff, 32, L"Main Loop Error %d\n", Result);
			OutputDebugString (Buff);
			GameShutdown ();

			return Result;
		}
	}

	GameShutdown ();

	return 0;
}