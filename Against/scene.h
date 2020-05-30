#pragma once

#include "vk_asset.h"
#include "error.h"
#include "scene_graphics.h"
#include "scene_physics.h"
#include "actor.h"

#include <Windows.h>

typedef struct _scene_obj
{
    scene_graphics_obj* graphics;
    scene_physics_obj* physics;

    skinned_actor* skinned_actors;
    size_t num_skinned_actors;

    static_actor* static_actors;
    size_t num_static_actors;
} scene_obj;

AGAINST_RESULT scene_import_image (const char* image_name);
AGAINST_RESULT scene_import_mesh (const char* mesh_name);

AGAINST_RESULT scene_commit_assets (void);

AGAINST_RESULT scene_init (const char* partial_folder_path, scene_obj* scene_obj);
AGAINST_RESULT scene_process_keyboard_input (WPARAM w_param, LPARAM l_param);
AGAINST_RESULT scene_main_loop (void);
void scene_shutdown ();
