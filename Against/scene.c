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
    CHECK_AGAINST_RESULT (scene_graphics_init (partial_folder_path, scene_obj->graphics), result);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_process_keyboard_input (WPARAM w_param, LPARAM l_param)
{
    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_main_loop ()
{
    AGAINST_RESULT result = AGAINST_SUCCESS;
    CHECK_AGAINST_RESULT (scene_graphics_main_loop (), result);
    return AGAINST_SUCCESS;
}

void scene_shutdown (scene_obj* scene_obj)
{
    scene_graphics_shutdown (scene_obj->graphics);
    utils_free (scene_obj->graphics);
}
