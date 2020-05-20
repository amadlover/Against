#include "scene.h"
#include "gltf.h"
#include "error.h"
#include "utils.h"
#include "vk_utils.h"
#include "common_graphics.h"

void scene_graphics_cleanpup_data (scene_graphics_obj* obj)
{
    OutputDebugString (L"scene_graphics_cleanpup_data\n");

    if (obj)
    {
        if (obj->images)
        {
            for (size_t i = 0; i < obj->num_images; ++i)
            {
                if (obj->images[i] != VK_NULL_HANDLE)
                {
                    vkDestroyImage (graphics_device, obj->images[i], NULL);
                }
                if (obj->image_views[i] != VK_NULL_HANDLE)
                {
                    vkDestroyImageView (graphics_device, obj->image_views[i], NULL);
                }
            }

            utils_free (obj->images);
            utils_free (obj->image_views);
            obj->num_images = 0;
        }

        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, obj->vb_ib, obj->vb_ib_memory);
        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, obj->bone_buffer, obj->bone_buffer_memory);
        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, obj->anim_buffer, obj->anim_buffer_memory);
        vkFreeMemory (graphics_device, obj->images_memory, NULL);

        vkDestroyDescriptorPool (graphics_device, obj->descriptor_pool, NULL);

        utils_free (obj->skinned_graphics_primitives);
        obj->num_skinned_graphics_primitives = 0;
        utils_free (obj->materials);
        obj->num_materials = 0;

        for (size_t s = 0; s < obj->num_skins; ++s)
        {
            utils_free (obj->skins[s].animations);
            obj->skins[s].num_animations = 0;
            utils_free (obj->skins[s].skinned_meshes);
            obj->skins[s].skinned_meshes = 0;
        }

        for (size_t a = 0; a < obj->num_animations; ++a)
        {
            utils_free ((obj->animations + a)->frame_data_offsets);
        }
        utils_free (obj->animations);
        obj->num_animations = 0;
        utils_free (obj->skins);
        obj->num_skins = 0;

        for (size_t m = 0; m < obj->num_skinned_meshes; ++m)
        {
            utils_free (obj->skinned_meshes[m].opaque_graphics_primitives);
            obj->skinned_meshes[m].num_opaque_graphics_primitives = 0;
            utils_free (obj->skinned_meshes[m].alpha_graphics_primitives);
            obj->skinned_meshes[m].num_alpha_graphics_primitives = 0;
            utils_free (obj->skinned_meshes[m].blend_graphics_primitives);
            obj->skinned_meshes[m].num_blend_graphics_primitives = 0;
        }

        utils_free (obj->skinned_meshes);
        obj->num_skinned_meshes = 0;

        for (size_t m = 0; m < obj->num_static_meshes; ++m)
        {
            utils_free (obj->static_meshes[m].opaque_graphics_primitives);
            obj->static_meshes[m].num_opaque_graphics_primitives = 0;
            utils_free (obj->static_meshes[m].alpha_graphics_primitives);
            obj->static_meshes[m].num_alpha_graphics_primitives = 0;
            utils_free (obj->static_meshes[m].blend_graphics_primitives);
            obj->static_meshes[m].num_blend_graphics_primitives = 0;
        }

        utils_free (obj->static_meshes);
        obj->num_static_meshes = 0;
    }
}

void scene_physics_cleanpup_data (scene_physics_obj* obj)
{

}

AGAINST_RESULT scene_init (const char* partial_folder_path, scene_obj* scene_obj)
{
    AGAINST_RESULT result = AGAINST_SUCCESS;
    scene_obj->graphics = (scene_graphics_obj*)utils_calloc (1, sizeof (scene_graphics_obj));
    scene_obj->physics = (scene_physics_obj*)utils_calloc (1, sizeof (scene_physics_obj));
    
    CHECK_AGAINST_RESULT (gltf_import_scene_data_from_files_from_folder (partial_folder_path, scene_obj), result);
    CHECK_AGAINST_RESULT (scene_graphics_init (scene_obj->graphics), result);
    CHECK_AGAINST_RESULT (scene_physics_init (scene_obj->physics), result);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_process_keyboard_input (WPARAM w_param, LPARAM l_param)
{
    return AGAINST_SUCCESS;
}

AGAINST_RESULT scene_main_loop ()
{
    AGAINST_RESULT result = AGAINST_SUCCESS;
    CHECK_AGAINST_RESULT (scene_graphics_main_loop (), result);
    return AGAINST_SUCCESS;
}

void scene_shutdown (scene_obj* scene_obj)
{
    scene_graphics_shutdown (scene_obj->graphics);
    scene_graphics_cleanpup_data (scene_obj->graphics);

    scene_physics_shutdown (scene_obj->physics);
    scene_physics_cleanpup_data (scene_obj->physics);
    
    utils_free (scene_obj->graphics);
    utils_free (scene_obj->physics);
}
