#pragma once

#include "asset.h"
#include "error.h"

#include <Windows.h>

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

    vk_skeletal_graphics_primitive* skeletal_graphics_primitives;
    size_t skeletal_graphics_primitives_count;

    vk_skeletal_physics_primitive* skeletal_physics_primitives;
    size_t skeletal_physics_primitives_count;

    vk_static_mesh* static_meshes;
    size_t static_meshes_count;

    vk_static_graphics_primitive* static_graphics_primitives;
    size_t static_graphics_primitives_count;

    vk_static_physics_primitive* static_physics_primitives;
    size_t static_physics_primitives_count;

    vk_material* materials;
    size_t materials_count;

    vk_skin* skins;
    size_t skins_count;

    vk_animation* animations;
    size_t animations_count;

    VkDescriptorPool descriptor_pool;
} scene_asset_data;

AGAINST_RESULT scene_import_data (const char* partial_folder_path, scene_asset_data** scene_data);
void scene_cleanpup_data (scene_asset_data* scene_data);

AGAINST_RESULT scene_init ();
AGAINST_RESULT scene_process_keyboard_input (WPARAM w_param, LPARAM l_param);
AGAINST_RESULT scene_main_loop ();
void scene_shutdown ();
