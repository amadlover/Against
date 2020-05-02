#include "scene.h"
#include "gltf.h"
#include "error.h"
#include "utils.h"
#include "vk_utils.h"
#include "common_graphics.h"

int import_scene_data (const char* partial_folder_path, scene_asset_data** scene_data)
{
    AGAINSTRESULT result;
    CHECK_AGAINST_RESULT (import_gltf_files_from_folder (partial_folder_path, scene_data), result);

    return 0;
}

void cleanup_scene_data (scene_asset_data* scene_data)
{
    OutputDebugString (L"cleanup_scene_data\n");

    if (scene_data)
    {
        if (scene_data->images)
        {
            for (size_t i = 0; i < scene_data->images_count; ++i)
            {
                if (scene_data->images[i] != VK_NULL_HANDLE)
                {
                    vkDestroyImage (graphics_device, scene_data->images[i], NULL);
                }
                if (scene_data->image_views[i] != VK_NULL_HANDLE)
                {
                    vkDestroyImageView (graphics_device, scene_data->image_views[i], NULL);
                }
            }

            utils_free (scene_data->images);
            utils_free (scene_data->image_views);
            scene_data->images_count = 0;
        }

        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, scene_data->vb_ib, scene_data->vb_ib_memory);
        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, scene_data->bone_buffer, scene_data->bone_buffer_memory);
        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, scene_data->anim_buffer, scene_data->anim_buffer_memory);
        vkFreeMemory (graphics_device, scene_data->images_memory, NULL);

        vkDestroyDescriptorPool (graphics_device, scene_data->descriptor_pool, NULL);

        utils_free (scene_data->graphics_primitives);
        scene_data->graphics_primitives_count = 0;
        utils_free (scene_data->materials);
        scene_data->materials_count = 0;

        for (size_t s = 0; s < scene_data->skins_count; ++s)
        {
            utils_free (scene_data->skins[s].animations);
            scene_data->skins[s].animations_count = 0;
            utils_free (scene_data->skins[s].opaque_graphics_primitives);
            scene_data->skins[s].opaque_graphics_primitives_count = 0;
            utils_free (scene_data->skins[s].alpha_graphics_primitives);
            scene_data->skins[s].alpha_graphics_primitives_count = 0;
            utils_free (scene_data->skins[s].blend_graphics_primitives);
            scene_data->skins[s].blend_graphics_primitives = 0;
        }

        utils_free (scene_data->animations);
        scene_data->animations_count = 0;
        utils_free (scene_data->skins);
        scene_data->skins_count = 0;
        utils_free (scene_data->skeletal_meshes);
        scene_data->skeletal_meshes_count = 0;
        utils_free (scene_data);
    }
}