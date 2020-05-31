#pragma once

#include "scene.h"
#include "error.h"

AGAINST_RESULT gltf_import_scene_data_from_files_from_folder (const char* partial_folder_path, scene* out_data);
