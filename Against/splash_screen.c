#include "splash_screen.h"

#include "event.h"
#include "asset.h"
#include "utils.h"
#include "error.h"

asset_mesh* meshes;
uint32_t meshes_count;

int splash_screen_init ()
{
    OutputDebugString (L"splash_screen_init\n");

    char full_file_path[256];
    get_full_file_path (full_file_path, "\\UIElements\\SplashScreen\\SplashScreen.gltf");

    int result = import_asset_meshes (full_file_path, &meshes, &meshes_count);

    if (result != 0) 
    {
        return result;
    }

    return 0;
}

int splash_screen_process_keyboard_input (WPARAM wParam, LPARAM lParam)
{
    int result = 0;

    switch (wParam)
    {
    case VK_ESCAPE:
        result = event_go_to_scene_fp (e_scene_type_main_menu);

        if (result != 0) 
        {
            return result;
        }

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

    destroy_asset_meshes (meshes, meshes_count);
    return 0;
}