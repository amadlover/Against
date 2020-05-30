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

scene_graphics* scene_get_graphics (scene* scene);
scene_physics* scene_get_physics (scene* scene);

void scene_shutdown ();
