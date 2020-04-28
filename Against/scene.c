#include "scene.h"
#include "utils.h"
#include "vk_utils.h"
#include "common_graphics.h"

void cleanup_scene_data (scene_asset_data* scene_data)
{
    OutputDebugString (L"cleanup_gltf_data\n");

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
        vkFreeMemory (graphics_device, scene_data->images_memory, NULL);

        vkDestroyDescriptorPool (graphics_device, scene_data->descriptor_pool, NULL);

        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, scene_data->bone_buffer, scene_data->bone_buffer_memory);

        utils_my_free (scene_data->graphics_primitives);
        utils_my_free (scene_data->materials);
        utils_my_free (scene_data->skins);
        utils_my_free (scene_data->animations);

        for (size_t m = 0; m < scene_data->skeletal_meshes_count; ++m)
        {
            utils_my_free (scene_data->skeletal_meshes[m].opaque_graphics_primitives);
            utils_my_free (scene_data->skeletal_meshes[m].alpha_graphics_primitives);
            utils_my_free (scene_data->skeletal_meshes[m].blend_graphics_primitives);
        }

        utils_my_free (scene_data->skeletal_meshes);
        utils_my_free (scene_data);
    }
}