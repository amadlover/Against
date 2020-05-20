#pragma once

#include "error.h"
#include "vk_asset.h"

typedef struct _scene_graphics_obj
{
    VkBuffer vb_ib;
    VkDeviceMemory vb_ib_memory;

    VkImage* images;
    VkImageView* image_views;
    size_t num_images;
    VkDeviceMemory images_memory;

    VkBuffer bone_buffer;
    VkDeviceMemory bone_buffer_memory;

    VkBuffer anim_buffer;
    VkDeviceMemory anim_buffer_memory;

    vk_skinned_mesh* skinned_meshes;
    size_t num_skinned_meshes;

    vk_skinned_primitive* skinned_graphics_primitives;
    size_t num_skinned_graphics_primitives;

    vk_static_mesh* static_meshes;
    size_t num_static_meshes;

    vk_static_primitive* static_graphics_primitives;
    size_t num_static_graphics_primitives;

    vk_material* materials;
    size_t num_materials;

    vk_skin* skins;
    size_t num_skins;

    vk_animation* animations;
    size_t num_animations;

    VkDescriptorPool descriptor_pool;
} scene_graphics_obj;

AGAINST_RESULT scene_graphics_init (scene_graphics_obj* scene_graphics_data);
AGAINST_RESULT scene_graphics_main_loop (void);
void scene_graphics_shutdown (scene_graphics_obj* scene_graphics_data);