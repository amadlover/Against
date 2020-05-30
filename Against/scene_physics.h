#pragma once

#include "phy_asset.h"
#include "error.h"

typedef struct _scene_physics
{
    size_t x;
} scene_physics;

AGAINST_RESULT scene_physics_init (scene_physics* out_data);
AGAINST_RESULT scene_physics_main_loop (void);
void scene_physics_shutdown (scene_physics* out_data);