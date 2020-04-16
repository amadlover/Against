#pragma once

#include "asset.h"

#include <vulkan/vulkan.h>

typedef struct 
{
    VkBuffer vb_ib;
    VkDeviceMemory vb_ib_memory;

    VkImage* images;
    VkImageView* image_views;
    size_t images_count;
    VkDeviceMemory images_memory;

    vk_material* materials;
    size_t materials_count;

} gltf_asset_data;

int import_gltf_files_from_folder (const char* partial_folder_path, gltf_asset_data** out_gltf_data);