#include "test_scene.h"
#include "game.h"
#include "gltf.h"
#include "graphics_pipeline.h"
#include "asset.h"
#include "utils.h"

#include <stb_image.h>

gltf_asset_data* asset_data = NULL;

int test_scene_init (HINSTANCE h_instnace, HWND h_wnd)
{
    OutputDebugString (L"test_scene_init\n");
    
    AGAINSTRESULT result;

    asset_data = (gltf_asset_data*)my_calloc (1, sizeof (gltf_asset_data));
    CHECK_AGAINST_RESULT (import_gltf_files_from_folder ("", &asset_data), result);

    return 0;
}

int test_scene_process_keyboard_input (WPARAM w_param, LPARAM l_param)
{
    OutputDebugString (L"test_scene_process_keyboard_input\n");
    
    AGAINSTRESULT result;

    switch (w_param)
    {
    case VK_ESCAPE:
        CHECK_AGAINST_RESULT (game_set_current_scene (e_scene_type_test), result);
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
    
    if (asset_data)
    {
        my_free (asset_data);
    }
}