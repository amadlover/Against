#pragma once

#include "asset.h"

#include <vulkan/vulkan.h>

typedef struct 
{
    VkBuffer vb_ib;
    VkDeviceMemory vb_ib_memory;

    VkImage* images;
    size_t images_count;
    VkDeviceMemory images_memory;
} gltf_asset_data;

int import_gltf_file (const char* partial_file_path, gltf_asset_data** out_gltf_data);
int import_gltf_files_from_folder (const char* partial_folder_path, gltf_asset_data** out_gltf_data);