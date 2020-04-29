#include "scene.h"
#include "utils.h"
#include "vk_utils.h"
#include "common_graphics.h"

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

            utils_my_free (scene_data->images);
            utils_my_free (scene_data->image_views);
        }

        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, scene_data->vb_ib, scene_data->vb_ib_memory);
        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, scene_data->bone_buffer, scene_data->bone_buffer_memory);
        vkFreeMemory (graphics_device, scene_data->images_memory, NULL);

        vkDestroyDescriptorPool (graphics_device, scene_data->descriptor_pool, NULL);

        utils_my_free (scene_data->graphics_primitives);
        utils_my_free (scene_data->materials);

        for (size_t s = 0; s < scene_data->skins_count; ++s)
        {
            utils_my_free (scene_data->skins[s].animations);
            utils_my_free (scene_data->skins[s].opaque_graphics_primitives);
            utils_my_free (scene_data->skins[s].alpha_graphics_primitives);
            utils_my_free (scene_data->skins[s].blend_graphics_primitives);
        }

        utils_my_free (scene_data->skins);
        utils_my_free (scene_data->skeletal_meshes);
        utils_my_free (scene_data);
    }
}