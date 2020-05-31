#include "test_scene.h"
#include "game.h"
#include "graphics_pipeline.h"
#include "common_graphics.h"
#include "vk_asset.h"
#include "utils.h"
#include "scene.h"
#include "vk_utils.h"

AGAINST_RESULT test_scene_init (void)
{
    OutputDebugString (L"test_scene_init\n");

    AGAINST_RESULT result = AGAINST_SUCCESS;

    CHECK_AGAINST_RESULT (scene_init (), result);

    CHECK_AGAINST_RESULT (scene_import_mesh ("PlayerShip"), result);
    CHECK_AGAINST_RESULT (scene_import_mesh ("LargeAsteroid"), result);
    CHECK_AGAINST_RESULT (scene_import_mesh ("SmallAsteroid"), result);

    CHECK_AGAINST_RESULT (scene_commit_assets (), result);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT test_scene_process_keyboard_input (WPARAM w_param, LPARAM l_param)
{
    OutputDebugString (L"test_scene_process_keyboard_input\n");
    
    AGAINST_RESULT result = AGAINST_SUCCESS;

    switch (w_param)
    {
    case VK_ESCAPE:
        CHECK_AGAINST_RESULT (game_set_current_scene (e_scene_type_test), result);
        break;

    default:
        break;
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT test_scene_main_loop (void)
{
    AGAINST_RESULT result = AGAINST_SUCCESS;

    CHECK_AGAINST_RESULT (scene_main_loop (), result);

    return AGAINST_SUCCESS;
}

void test_scene_shutdown (void)
{
    OutputDebugString (L"test_scene_shutdown\n");

    scene_shutdown ();
}