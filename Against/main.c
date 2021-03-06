#include <Windows.h>

#include <stdlib.h>

#include "game.h"
#include "log.h"

LRESULT CALLBACK WindowProc (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_COMMAND:
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
		game_process_keyboard_input (wParam, lParam);

		break;

	case WM_LBUTTONDOWN:
		game_process_left_mouse_click ();

		break;

	case WM_MBUTTONDOWN:
		game_process_middle_mouse_click ();

		break;

	case WM_RBUTTONDOWN:
		game_process_right_mouse_click ();

		break;

	case WM_MOUSEMOVE:
		game_process_mouse_movement (wParam, lParam);

		break;


	default:
		break;
	}

	return DefWindowProc (hWnd, msg, wParam, lParam);
}

int WINAPI wWinMain (_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE previous_instance, _In_ PWSTR cmd_line, _In_ int cmd_show)
{
	WNDCLASS wc = { 0 };

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = L"Against";
	wc.hCursor = LoadCursor (hInstance, IDC_ARROW);

	if (!RegisterClass (&wc))
	{
		return EXIT_FAILURE;
	}

	HWND hWnd = CreateWindow (L"Against", L"Against", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return EXIT_FAILURE;
	}

	AGAINST_RESULT result = game_init (hInstance, hWnd);

	if (result != AGAINST_SUCCESS)
	{
		log_error (result);
		game_shutdown ();

		return EXIT_FAILURE;
	}

	ShowWindow (hWnd, cmd_show);
	UpdateWindow (hWnd);

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

		if (result != AGAINST_SUCCESS)
		{
			log_error (result);
			game_shutdown ();

			return EXIT_FAILURE;
		}
	}

	game_shutdown ();
	DestroyWindow (hWnd);

	return EXIT_SUCCESS;
}