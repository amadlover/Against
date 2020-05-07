#include "test_scene.h"
#include "game.h"
#include "graphics_pipeline.h"
#include "common_graphics.h"
#include "asset.h"
#include "utils.h"
#include "scene.h"
#include "vk_utils.h"

scene_asset_data* asset_data = NULL;
vk_skeletal_opaque_graphics_pipeline* skeletal_opaque_graphics_pipeline = NULL;

int test_scene_init (HINSTANCE h_instnace, HWND h_wnd)
{
    OutputDebugString (L"test_scene_init\n");
    
    AGAINSTRESULT result;

    CHECK_AGAINST_RESULT (import_scene_data ("", &asset_data), result);
    CHECK_AGAINST_RESULT (create_skeletal_opaque_graphics_pipeline (asset_data, &skeletal_opaque_graphics_pipeline), result);
    
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

int draw_skybox ()
{
    OutputDebugString (L"draw_skybox\n");
    return 0;
}

int draw_opaque_skeletal_meshes ()
{
    OutputDebugString (L"draw_opaque_skeletal_meshes\n");

    return 0;
}

int update_command_buffers ()
{
    OutputDebugString (L"test_scene_update_command_buffers\n");

    AGAINSTRESULT result;
    CHECK_AGAINST_RESULT (draw_skybox (), result);
    CHECK_AGAINST_RESULT (draw_opaque_skeletal_meshes (), result);

    return 0;
}

int tick ()
{
    return 0;
}

int present ()
{
    return 0;
}

int test_scene_main_loop ()
{
    AGAINSTRESULT result;

    CHECK_AGAINST_RESULT (tick (), result);
    CHECK_AGAINST_RESULT (present (), result);

    return 0;
}

void test_scene_exit ()
{
    OutputDebugString (L"test_scene_exit\n");

    destroy_skeletal_opaque_graphics_pipeline (skeletal_opaque_graphics_pipeline);
    cleanup_scene_data (asset_data);
}