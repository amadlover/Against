#pragma once

#include "vk_asset.h"
#include "scene.h"
#include "error.h"
#include <vulkan/vulkan.h>

AGAINST_RESULT gltf_import_files_from_folder (const char* partial_folder_path, scene_graphics_obj* out_gltf_data);
AGAINST_RESULT gltf_import_file (const char* file_path);
