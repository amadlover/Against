#include "actor.h"

struct _skinned_actor {
    char name[1024];
    
    vk_skin* skin;
    
    phy_skinned_primitive** physics_primitives;
    size_t num_physics_primitives;
};

struct _static_actor {
    char name[1024];

    vk_static_mesh* static_meshes;
    size_t num_static_meshes;

    phy_static_primitive** physics_primitives;
    size_t num_physics_primitives;
};