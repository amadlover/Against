#include "graphics_pipeline.h"
#include "utils.h"

int create_opaque_graphics_pipeline (scene_asset_data* scene_data, vk_skeletal_opaque_graphics_pipeline** out_graphics_pipeline)
{
    OutputDebugString (L"create_opaque_graphics_pipeline\n");

    *out_graphics_pipeline = (vk_skeletal_opaque_graphics_pipeline*)my_calloc (1, sizeof (vk_skeletal_opaque_graphics_pipeline));

    for (size_t m = 0; m < scene_data->materials_count; ++m)
    {
        vk_skeletal_material* material = scene_data->materials + m;

        if (strstr (material->name, "opaque") != NULL)
        {
            if ((*out_graphics_pipeline)->materials == NULL)
            {
                (*out_graphics_pipeline)->materials = (vk_skeletal_material**)my_calloc (1, sizeof (vk_skeletal_material*));
            }
            else
            {
                (*out_graphics_pipeline)->materials = (vk_skeletal_material**)my_realloc_zero ((*out_graphics_pipeline)->materials, sizeof (vk_skeletal_material*) * (*out_graphics_pipeline)->materials_count, sizeof (vk_skeletal_material*) * ((*out_graphics_pipeline)->materials_count + 1));
            }

            (*out_graphics_pipeline)->materials[(*out_graphics_pipeline)->materials_count] = material;

            ++(*out_graphics_pipeline)->materials_count;
        }
    }

    return 0;
}

void destroy_opaque_graphics_pipeline (vk_skeletal_opaque_graphics_pipeline* graphics_pipeline)
{
    OutputDebugString (L"destroy_opaque_graphics_pipeline\n");

    my_free (graphics_pipeline->materials);
    my_free (graphics_pipeline);
}