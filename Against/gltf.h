#pragma once

#include "asset.h"
#include "scene.h"
#include "error.h"
#include <vulkan/vulkan.h>

AGAINST_RESULT import_gltf_files_from_folder (const char* partial_folder_path, scene_asset_data** out_gltf_data);
AGAINST_RESULT import_gltf_file (const char* file_path);
