#pragma once

#include "asset.h"

#include <vulkan/vulkan.h>

typedef struct 
{
    vk_skeletal_mesh* skeletal_meshes;
    size_t skeletal_mesh_count;

    VkBuffer vb_ib;
    VkDeviceMemory vb_ib_memory;

    VkImage* images;
    VkImageView* image_views;
    size_t images_count;
    VkDeviceMemory images_memory;

    vk_material* materials;
    size_t materials_count;

    vk_skin* skins;
    size_t skins_count;

    vk_animation* animations;
    size_t animations_count;
} gltf_asset_data;

int import_gltf_files_from_folder (const char* partial_folder_path, gltf_asset_data** out_gltf_data);
void cleanup_gltf_data (gltf_asset_data* gltf_data);

int import_gltf_file (const char* file_path);