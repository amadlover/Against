#pragma once

#include "asset.h"

typedef struct _scene_asset_data
{
    VkBuffer vb_ib;
    VkDeviceMemory vb_ib_memory;

    VkImage* images;
    VkImageView* image_views;
    size_t images_count;
    VkDeviceMemory images_memory;

    VkBuffer bone_buffer;
    VkDeviceMemory bone_buffer_memory;

    VkBuffer anim_buffer;
    VkDeviceMemory anim_buffer_memory;

    vk_skeletal_mesh* skeletal_meshes;
    size_t skeletal_meshes_count;

    vk_skeletal_graphics_primitive* graphics_primitives;
    size_t graphics_primitives_count;

    vk_skeletal_material* materials;
    size_t materials_count;

    vk_skin* skins;
    size_t skins_count;

    vk_animation* animations;
    size_t animations_count;

    VkDescriptorPool descriptor_pool;
} scene_asset_data;

int import_scene_data (const char* partial_folder_path, scene_asset_data** scene_data);
void cleanup_scene_data (scene_asset_data* scene_data);