#pragma once

#include "phy_asset.h"
#include "vk_asset.h"

typedef struct _skinned_actor {
    char name[1024];
    
    vk_skin* skin;
    
    phy_skinned_primitive** physics_primitives;
    size_t num_physics_primitives;
}skinned_actor;

typedef struct _static_actor {
    char name[1024];

    vk_static_mesh* static_meshes;
    size_t num_static_meshes;

    phy_static_primitive** physics_primitives;
    size_t num_physics_primitives;
} static_actor;

skinned_actor* actor_make_skinned_actor_obj ();

size_t actor_get_skinned_actor_size ();
size_t actor_get_static_actor_size ();