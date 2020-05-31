#pragma once

#include "phy_asset.h"
#include "vk_asset.h"

typedef struct _skinned_actor skinned_actor;
typedef struct _static_actor static_actor;

size_t actor_get_skinned_actor_size ();
size_t actor_get_static_actor_size ();