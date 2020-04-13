#include "test_scene.h"
#include "game.h"
#include "gltf.h"
#include "graphics_pipeline.h"
#include "asset.h"
#include "utils.h"

#include <stb_image.h>

gltf_scene_data* scene_asset_data = NULL;

int test_scene_init (HINSTANCE h_instnace, HWND h_wnd)
{
    OutputDebugString (L"test_scene_init\n");
    
    scene_asset_data = (gltf_scene_data*)my_calloc (1, sizeof (gltf_scene_data));
    import_gltf_files_from_folder ("", &scene_asset_data);

    return 0;
}

int test_scene_process_keyboard_input (WPARAM w_param, LPARAM l_param)
{
    OutputDebugString (L"test_scene_process_keyboard_input\n");
    switch (w_param)
    {
    case VK_ESCAPE:
        game_set_current_scene (e_scene_type_test);
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
    
    if (scene_asset_data)
    {
        my_free (scene_asset_data->animations);

        for (size_t i = 0; i < scene_asset_data->images_count; ++i)
        {
            free_image_data ((scene_asset_data->images + i)->pixels);
        }

        my_free (scene_asset_data->images);
        my_free (scene_asset_data->materials);
        my_free (scene_asset_data->samplers);
        my_free (scene_asset_data->skeletal_meshes);
        my_free (scene_asset_data->static_meshes);
        my_free (scene_asset_data->skins);
        my_free (scene_asset_data->textures);

        my_free (scene_asset_data);
    }
}