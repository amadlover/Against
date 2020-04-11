#pragma once

#include "asset.h"

typedef struct 
{
    asset_mesh* meshes;
} gltf_scene_data;

int import_gltf_file (const char* partial_file_path);
void import_gltf_files_from_folder (const char* partial_folder_path);