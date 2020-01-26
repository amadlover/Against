#include <Windows.h>

#include <stdio.h>

#include "game.h"
#include "log.h"
#include "utils.h"

LRESULT CALLBACK WindowProc 
(
	HWND hWnd, 
	UINT msg, 
	WPARAM wParam, 
	LPARAM lParam
)
{
	switch (msg)
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

	return DefWindowProc (hWnd, msg, wParam, lParam);
}

#include "test.h"

int WINAPI wWinMain (_In_ HINSTANCE hInstance, 
						_In_opt_ HINSTANCE previous_instance, 
						_In_ PWSTR cmd_line, 
						_In_ int cmd_show)
{
	int result = log_init ();
	if (result != 0)
	{
		log_error (result);
	}

	result = log_info ("TEST", "Testing....");

	if (result != 0)
	{
		log_error (result);
	}

	result = log_info ("TEST", "Still Testing....");

	if (result != 0)
	{
		log_error (result);
	}

	result = log_exit ();

	if (result != 0) 
	{
		log_error (result);
	}

	WNDCLASS wc = { 0 };

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"Against";
	wc.hCursor = LoadCursor (hInstance, IDC_ARROW);

	if (!RegisterClass (&wc))
	{
		return 0;
	}

	HWND hWnd = CreateWindow 
	(
		L"Against", 
		L"Against", 
		WS_OVERLAPPEDWINDOW, 
		CW_USEDEFAULT, 
		CW_USEDEFAULT, 
		1280, 
		720, 
		NULL, 
		NULL, 
		hInstance, 
		NULL
	);

	if (!hWnd)
	{
		return -1;
	}

	result = game_init (hInstance, hWnd);

	ShowWindow (hWnd, cmd_show);
	UpdateWindow (hWnd);

	if (result != 0)
	{
		wchar_t Buff[8];
		swprintf_s (Buff, 8, L"Init ");
		OutputDebugString (Buff);

		log_error (result);

		game_exit ();

		return result;
	}

	MSG msg;
	ZeroMemory (&msg, sizeof (msg));

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}

		result = game_main_loop ();

		if (result != 0)
		{
			wchar_t Buff[16];
			swprintf_s (Buff, 16, L"Main Loop ");
			OutputDebugString (Buff);

			log_error (result);

			game_exit ();

			return result;
		}
	}

	game_exit ();

	DestroyWindow (hWnd);

	return 0;
}