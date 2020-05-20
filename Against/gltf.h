#pragma once

#include "vk_asset.h"
#include "scene.h"
#include "error.h"
#include <vulkan/vulkan.h>

AGAINST_RESULT gltf_import_scene_data_from_files_from_folder (const char* partial_folder_path, scene_obj* out_data);
