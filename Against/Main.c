#include <Windows.h>

#include <stdio.h>

#include "game.h"
#include "error.h"
#include "utils.h"

LRESULT CALLBACK WindowProc (HWND WindowHandle, 
								UINT Msg, 
								WPARAM wParam, 
								LPARAM lParam)
{
	switch (Msg)
	{
	case WM_COMMAND:
		switch (wParam)
		{
		case 1:
			OutputDebugString (L"New Game\n");
			break;

		case 2:
			PostQuitMessage (0);
			break;

		default:
			break;
		}
		break;

	case WM_QUIT:
		OutputDebugString (L"WM_QUIT\n");

		break;

	case WM_DESTROY:
		PostQuitMessage (0);

		break;

	case WM_CLOSE:
		PostQuitMessage (0);

		break;

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

	return DefWindowProc (WindowHandle, Msg, wParam, lParam);
}

#include "test.h"

int WINAPI wWinMain (_In_ HINSTANCE HInstance, 
						_In_opt_ HINSTANCE PreviousHInstance, 
						_In_ PWSTR CmdLine, 
						_In_ int CmdShow)
{
	WNDCLASS WC = { 0 };

	WC.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	WC.lpfnWndProc = WindowProc;
	WC.hInstance = HInstance;
	WC.lpszClassName = L"Against";
	WC.hCursor = LoadCursor (HInstance, IDC_ARROW);

	if (!RegisterClass (&WC))
	{
		return 0;
	}

	HWND WindowHandle = CreateWindow (L"Against", 
										L"Against", 
										WS_OVERLAPPEDWINDOW, 
										CW_USEDEFAULT, 
										CW_USEDEFAULT, 
										1280, 
										720, 
										NULL, 
										NULL, 
										HInstance, 
										NULL);

	if (!WindowHandle)
	{
		return -1;
	}

	int Result = GameInit (HInstance, WindowHandle);

	ShowWindow (WindowHandle, CmdShow);
	UpdateWindow (WindowHandle);

	if (Result != 0)
	{
		wchar_t Buff[8];
		swprintf_s (Buff, 8, L"Init ");
		OutputDebugString (Buff);

		LogError (Result);

		GameShutdown ();

		return Result;
	}

	MSG Msg;
	ZeroMemory (&Msg, sizeof (Msg));

	while (Msg.message != WM_QUIT)
	{
		if (PeekMessage (&Msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage (&Msg);
			DispatchMessage (&Msg);
		}

		Result = GameMainLoop ();

		if (Result != 0)
		{
			wchar_t Buff[16];
			swprintf_s (Buff, 16, L"Main Loop ");
			OutputDebugString (Buff);

			LogError (Result);

			GameShutdown ();

			return Result;
		}
	}

	GameShutdown ();

	DestroyWindow (WindowHandle);

	return 0;
}