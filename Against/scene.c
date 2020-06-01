#include "scene.h"
#include "gltf.h"
#include "error.h"
#include "utils.h"
#include "vk_utils.h"
#include "actor.h"
#include "common_graphics.h"

#include <stb_image.h>

char** image_names = NULL;
size_t num_image_names = 0;

char** mesh_names = NULL;
size_t num_mesh_names = 0;

scene* scene_obj = NULL;

AGAINST_RESULT scene_import_image (const char* image_name)
{
    OutputDebugString (L"scene_import_image\n");

    if (image_names == NULL)
    {
        image_names = (char**)utils_calloc (1, sizeof (char*));
    }
    else
    {
        image_names = (char**)utils_realloc_zero (image_names, sizeof (char*) * num_image_names, sizeof (char*) * (num_image_names + 1));
    }
    
    image_names[num_image_names] = (char*)utils_calloc (strlen (image_name) + 5, sizeof (char));
    strcpy (image_names[num_image_names], image_name);
    strcat (image_names[num_image_names], ".tga");

    ++num_image_names;

    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_import_mesh (const char* mesh_name)
{
    OutputDebugString (L"scene_import_mesh\n");

    if (mesh_names == NULL)
    {
        mesh_names = (char**)utils_calloc (1, sizeof (char*));
    }
    else
    {
        mesh_names = (char**)utils_realloc_zero (mesh_names, sizeof (char*) * num_mesh_names, sizeof (char*) * (num_mesh_names + 1));
    }

    mesh_names[num_mesh_names] = (char*)utils_calloc (strlen (mesh_name) + 6, sizeof (char));
    strcpy (mesh_names[num_mesh_names], mesh_name);
    strcat (mesh_names[num_mesh_names], ".gltf");

    ++num_mesh_names;

    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_commit_assets (void)
{
    OutputDebugString (L"scene_commit_assets\n");

    for (size_t i = 0; i < num_mesh_names; ++i)
    {
        char full_file_path[MAX_PATH];
        utils_get_full_file_path (mesh_names[i], full_file_path);
        wchar_t buff[MAX_PATH];
        swprintf (buff, MAX_PATH, L"%hs\n", full_file_path);
        OutputDebugString (buff);
    }

    utils_free (image_names);
    num_image_names = 0;
    utils_free (mesh_names);
    num_mesh_names = 0;

    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_init ()
{
    OutputDebugString (L"scene_init\n");
    AGAINST_RESULT result = AGAINST_SUCCESS;
    scene_obj = (scene*) utils_calloc (1, sizeof (scene));
    scene_obj->graphics = (scene_graphics*)utils_calloc (1, sizeof (scene_graphics));
    scene_obj->physics = (scene_physics*)utils_calloc (1, sizeof (scene_physics));
    
    /*CHECK_AGAINST_RESULT (gltf_import_scene_data_from_files_from_folder (partial_folder_path, scene_obj), result);
    CHECK_AGAINST_RESULT (scene_graphics_init (scene_obj->graphics), result);
    CHECK_AGAINST_RESULT (scene_physics_init (scene_obj->physics), result);*/

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

AGAINST_RESULT scene_spawn_skinned_actor (const char* name)
{
    if (scene_obj->skinned_actors == NULL)
    {
        scene_obj->skinned_actors = (skinned_actor*) utils_calloc (1, sizeof (skinned_actor));
    }
    else
    {
        scene_obj->skinned_actors = (skinned_actor*) utils_realloc_zero (scene_obj->skinned_actors, sizeof (skinned_actor) * scene_obj->num_skinned_actors, sizeof (skinned_actor) * (scene_obj->num_skinned_actors + 1));
    }
    
    skinned_actor* current_actor = scene_obj->skinned_actors + scene_obj->num_skinned_actors;

    current_actor = (skinned_actor*) utils_calloc (1, sizeof (skinned_actor));
    strcpy (current_actor->name, name);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_destroy_skinned_actor (skinned_actor* actor)
{
    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_spawn_static_actor (const char* name)
{
    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_destroy_static_actor (static_actor* actor)
{
    return AGAINST_SUCCESS;
}

void scene_shutdown ()
{
    scene_graphics_shutdown (scene_obj->graphics);
    utils_free (scene_obj->graphics);
    
    scene_physics_shutdown (scene_obj->physics);
    utils_free (scene_obj->physics);

    for (size_t a = 0; a < scene_obj->num_skinned_actors; ++a)
    {
        utils_free (scene_obj->skinned_actors + a);
    }
    utils_free (scene_obj->skinned_actors);
    scene_obj->num_skinned_actors = 0;
    
    for (size_t a = 0; a < scene_obj->num_static_actors; ++a)
    {
        utils_free (scene_obj->static_actors + a);
    }
    utils_free (scene_obj->static_actors);
    scene_obj->num_static_actors = 0;

    utils_free (scene_obj);
}
