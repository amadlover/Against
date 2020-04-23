#include "test_scene.h"
#include "game.h"
#include "gltf.h"
#include "graphics_pipeline.h"
#include "common_graphics.h"
#include "asset.h"
#include "utils.h"
#include "vk_utils.h"

scene_asset_data* asset_data = NULL;
vk_skeletal_opaque_graphics_pipeline* skeletal_opaque_graphics_pipeline = NULL;
vk_skeletal_alpha_graphics_pipeline* skeletal_alpha_graphics_pipeline = NULL;

int test_scene_init (HINSTANCE h_instnace, HWND h_wnd)
{
    OutputDebugString (L"test_scene_init\n");
    
    AGAINSTRESULT result;

    CHECK_AGAINST_RESULT (import_gltf_files_from_folder ("", &asset_data), result);
    CHECK_AGAINST_RESULT (create_opaque_graphics_pipeline (asset_data, &skeletal_opaque_graphics_pipeline), result);

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

    destroy_opaque_graphics_pipeline (skeletal_opaque_graphics_pipeline);
    cleanup_gltf_data (asset_data);
}