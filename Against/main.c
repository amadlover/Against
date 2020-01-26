#include <Windows.h>

#include <stdio.h>

#include "game.h"
#include "log.h"
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
		process_keyboard_input (wParam, lParam);

		break;

	case WM_LBUTTONDOWN:
		process_left_mouse_click ();

		break;

	case WM_MBUTTONDOWN:
		process_middle_mouse_click ();

		break;

	case WM_RBUTTONDOWN:
		process_right_mouse_click ();

		break;

	case WM_MOUSEMOVE:
		process_mouse_movement (wParam, lParam);

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
	int Result = log_init ();
	if (Result != 0)
	{
		log_error (Result);
	}

	Result = log_info ("TEST", "Testing....");

	if (Result != 0)
	{
		log_error (Result);
	}

	Result = log_info ("TEST", "Still Testing....");

	if (Result != 0)
	{
		log_error (Result);
	}

	Result = log_exit ();

	if (Result != 0) 
	{
		log_error (Result);
	}

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

	Result = game_init (HInstance, WindowHandle);

	ShowWindow (WindowHandle, CmdShow);
	UpdateWindow (WindowHandle);

	if (Result != 0)
	{
		wchar_t Buff[8];
		swprintf_s (Buff, 8, L"Init ");
		OutputDebugString (Buff);

		log_error (Result);

		game_exit ();

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

		Result = game_main_loop ();

		if (Result != 0)
		{
			wchar_t Buff[16];
			swprintf_s (Buff, 16, L"Main Loop ");
			OutputDebugString (Buff);

			log_error (Result);

			game_exit ();

			return Result;
		}
	}

	game_exit ();

	DestroyWindow (WindowHandle);

	return 0;
}