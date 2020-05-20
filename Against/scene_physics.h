#pragma once

#include "phy_asset.h"
#include "error.h"

typedef struct _scene_physics_obj
{
    size_t x;
} scene_physics_obj;

AGAINST_RESULT scene_physics_init (scene_physics_obj* out_data);
AGAINST_RESULT scene_physics_main_loop ();
void scene_physics_shutdown (scene_physics_obj* out_data);