#pragma once

#include "asset.h"

typedef struct 
{
    asset_static_mesh* static_meshes;
    size_t static_meshes_count;

    asset_skeletal_mesh* skeletal_meshes;
    size_t skeletal_meshes_count;

    asset_material* materials;
    size_t materials_count;

    asset_texture* textures;
    size_t textures_count;

    asset_sampler* samplers;
    size_t samplers_count;

    asset_image* images;
    size_t images_count;

    asset_skin* skins;
    size_t skins_count;

    asset_animation* animations;
    size_t animation_count;
} gltf_asset_data;

int import_gltf_file (const char* partial_file_path, gltf_asset_data** out_gltf_data);
int import_gltf_files_from_folder (const char* partial_folder_path, gltf_asset_data** out_gltf_data);