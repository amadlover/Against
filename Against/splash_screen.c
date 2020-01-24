#include "splash_screen.h"

#include "event.h"

int splash_screen_init ()
{
    OutputDebugString (L"splash_screen_init\n");
    return 0;
}

int splash_screen_process_keyboard_input (WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case VK_ESCAPE:
        event_go_to_scene_fp (e_scene_type_main_menu);
        break;
    default:
        break;
    }
    return 0;
}

int splash_screen_main_loop ()
{
    return 0;
}

int splash_screen_exit ()
{
    OutputDebugString (L"splash_screen_exit\n");
    return 0;
}