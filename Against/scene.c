#include "scene.h"
#include "gltf.h"
#include "error.h"
#include "utils.h"
#include "vk_utils.h"
#include "common_graphics.h"

AGAINST_RESULT scene_init (const char* partial_folder_path, scene_obj* scene_obj)
{
    AGAINST_RESULT result = AGAINST_SUCCESS;
    scene_obj->graphics = (scene_graphics_obj*)utils_calloc (1, sizeof (scene_graphics_obj));
    scene_obj->physics = (scene_physics_obj*)utils_calloc (1, sizeof (scene_physics_obj));
    
    CHECK_AGAINST_RESULT (gltf_import_scene_data_from_files_from_folder (partial_folder_path, scene_obj), result);
    CHECK_AGAINST_RESULT (scene_graphics_init (scene_obj->graphics), result);
    CHECK_AGAINST_RESULT (scene_physics_init (scene_obj->physics), result);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_process_keyboard_input (WPARAM w_param, LPARAM l_param)
{
    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_main_loop (void)
{
    AGAINST_RESULT result = AGAINST_SUCCESS;
    CHECK_AGAINST_RESULT (scene_graphics_main_loop (), result);
    return AGAINST_SUCCESS;
}

void scene_shutdown (scene_obj* scene_obj)
{
    scene_graphics_shutdown (scene_obj->graphics);
    utils_free (scene_obj->graphics);
    
    scene_physics_shutdown (scene_obj->physics);
    
    utils_free (scene_obj->physics);
}
