#include "main_menu.h"

#include "event.h"
#include "enums.h"

int main_menu_init ()
{
    OutputDebugString (L"main_menu_init\n");

    return 0;
}

int main_menu_process_keyboard_input (WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case VK_ESCAPE:
        event_go_to_scene_fp (e_scene_type_splash_screen);
        break;

    default:
        break;
    }
    return 0;
}

int main_menu_main_loop ()
{
    return 0;
}

int main_menu_exit ()
{
    OutputDebugString (L"main_menu_exit\n");

    return 0;
}