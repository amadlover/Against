#pragma once

#include "phy_asset.h"
#include "vk_asset.h"

typedef struct _skinned_actor
{
    vk_skin* skin;
    
    phy_skinned_primitive** physics_primitives;
    size_t num_physics_primitive;
} skinned_actor;

typedef struct _static_actor
{
    vk_static_mesh* static_meshes;
    size_t num_static_meshes;

    phy_static_primitive** physics_primitives;
    size_t num_physics_primitive;
} static_actor;