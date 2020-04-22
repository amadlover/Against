#pragma once

#include "asset.h"

#include <vulkan/vulkan.h>

int import_gltf_files_from_folder (const char* partial_folder_path, scene_asset_data** out_gltf_data);
void cleanup_gltf_data (scene_asset_data* gltf_data);

int import_gltf_file (const char* file_path);