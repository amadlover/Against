#pragma once

#include "enums.h"
#include <stdint.h>

#include <vulkan/vulkan.h>

typedef struct 
{
    size_t x;
} scene_asset_data;

int import_gltf_file (const char* partial_file_path, scene_asset_data** out_gltf_data);
int import_gltf_files_from_folder (const char* partial_folder_path, scene_asset_data** out_gltf_data);

int log_init ();
int log_info (const char* category, const char* message);
int log_error (AGAINSTRESULT Result);
int log_exit ();