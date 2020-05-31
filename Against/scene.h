#pragma once

#include "vk_asset.h"
#include "error.h"
#include "scene_graphics.h"
#include "scene_physics.h"
#include "actor.h"

#include <Windows.h>

typedef struct _scene scene;

AGAINST_RESULT scene_import_image (const char* image_name);
AGAINST_RESULT scene_import_mesh (const char* mesh_name);

AGAINST_RESULT scene_commit_assets (void);

AGAINST_RESULT scene_init (const char* partial_folder_path);
AGAINST_RESULT scene_process_keyboard_input (WPARAM w_param, LPARAM l_param);
AGAINST_RESULT scene_main_loop (void);

AGAINST_RESULT scene_spawn_skinned_actor (const char* name);
AGAINST_RESULT scene_destroy_skinned_actor (skinned_actor* actor);

AGAINST_RESULT scene_spawn_static_actor (static_actor* actor);
AGAINST_RESULT scene_destroy_static_actor (static_actor* actor);

scene_graphics* scene_get_graphics ();
scene_physics* scene_get_physics ();

void scene_shutdown ();
