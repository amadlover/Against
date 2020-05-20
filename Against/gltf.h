#pragma once

#include "vk_asset.h"
#include "scene.h"
#include "error.h"
#include <vulkan/vulkan.h>

AGAINST_RESULT gltf_import_graphics_from_files_from_folder (const char* partial_folder_path, scene_graphics_obj* out_gltf_data);
