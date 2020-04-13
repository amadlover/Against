#include "test_scene.h"
#include "game.h"
#include "gltf.h"
#include "graphics_pipeline.h"
#include "asset.h"

gltf_scene_data* scene_data;

int test_scene_init (HINSTANCE h_instnace, HWND h_wnd)
{
    OutputDebugString (L"test_scene_init\n");
    import_gltf_files_from_folder ("", scene_data);

    return 0;
}

int test_scene_process_keyboard_input (WPARAM w_param, LPARAM l_param)
{
    OutputDebugString (L"test_scene_process_keyboard_input\n");
    switch (w_param)
    {
    case VK_ESCAPE:
        game_set_current_scene (2);
        break;

    default:
        break;
    }

    return 0;
}

int test_scene_main_loop ()
{
    return 0;
}

void test_scene_exit ()
{
    OutputDebugString (L"test_scene_exit\n");
}