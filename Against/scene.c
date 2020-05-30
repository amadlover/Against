#include "scene.h"
#include "gltf.h"
#include "error.h"
#include "utils.h"
#include "vk_utils.h"
#include "common_graphics.h"

char** image_names = NULL;
size_t num_image_names = 0;

char** mesh_names = NULL;
size_t num_mesh_names = 0;

scene_obj* scene = NULL;

AGAINST_RESULT scene_import_image (const char* image_name)
{
    if (image_names == NULL)
    {
        image_names = (char**)utils_calloc (1, sizeof (char*));
    }
    else
    {
        image_names = (char**)utils_realloc_zero (image_names, sizeof (char*) * num_image_names, sizeof (char*) * (num_image_names + 1));
    }
    
    image_names[num_image_names] = (char*)utils_calloc (strlen (image_name), sizeof (char));
    strcpy (image_names[num_image_names], image_name);
    strcat (image_names[num_image_names], ".tga");

    ++num_image_names;

    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_import_mesh (const char* mesh_name)
{
    if (mesh_names == NULL)
    {
        mesh_names = (char**)utils_calloc (1, sizeof (char*));
    }
    else
    {
        mesh_names = (char**)utils_realloc_zero (mesh_names, sizeof (char*) * num_mesh_names, sizeof (char*) * (num_mesh_names + 1));
    }

    mesh_names[num_mesh_names] = (char*)utils_calloc (strlen (mesh_name), sizeof (char));
    strcpy (mesh_names[num_mesh_names], mesh_name);
    strcat (mesh_names[num_mesh_names], ".gltf");

    ++num_mesh_names;

    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_commit_assets (void)
{
    utils_free (image_names);
    num_image_names = 0;
    utils_free (mesh_names);
    num_mesh_names = 0;

    return AGAINST_SUCCESS;
}

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

void scene_shutdown ()
{
    scene_graphics_shutdown (scene->graphics);
    utils_free (scene->graphics);
    
    scene_physics_shutdown (scene->physics);
    
    utils_free (scene->physics);
    utils_free (scene);
}
