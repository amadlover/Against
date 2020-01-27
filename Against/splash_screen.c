#include "splash_screen.h"
#include "event.h"
#include "asset.h"
#include "utils.h"
#include "error.h"
#include "splash_screen_graphics.h"

asset_mesh* meshes;
uint32_t mesh_count;

int splash_screen_init ()
{
    OutputDebugString (L"splash_screen_init\n");

    char full_file_path[256];
    get_full_file_path (full_file_path, "\\UIElements\\SplashScreen\\SplashScreen.gltf");

    CHECK_AGAINST_RESULT (import_asset_meshes (full_file_path, &meshes, &mesh_count));
    CHECK_AGAINST_RESULT (splash_screen_graphics_init (meshes, mesh_count));

    return 0;
}

int splash_screen_process_keyboard_input (WPARAM wParam, LPARAM lParam)
{
    switch (wParam)
    {
    case VK_ESCAPE:
        CHECK_AGAINST_RESULT (event_go_to_scene_fp (e_scene_type_main_menu));

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

    destroy_asset_meshes (meshes, mesh_count);
    splash_screen_graphics_exit ();

    return 0;
}