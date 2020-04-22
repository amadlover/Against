#pragma once

#include "error.h"
#include "asset.h"

typedef struct
{
    vk_skeletal_material** materials;
    size_t materials_count;
} vk_skeletal_opaque_graphics_pipeline;

typedef struct
{
    vk_skeletal_material** materials;
    size_t materials_count;
} vk_skeletal_alpha_graphics_pipeline;

typedef struct
{
    vk_skeletal_material** materials;
    size_t materials_count;
} vk_skeletal_blend_graphics_pipeline;

int create_opaque_graphics_pipeline (scene_asset_data* scene_data, vk_skeletal_opaque_graphics_pipeline** out_graphics_pipeline);
void destroy_opaque_graphics_pipeline (vk_skeletal_opaque_graphics_pipeline* graphics_pipeline);