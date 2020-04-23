#include "graphics_pipeline.h"
#include "utils.h"
#include "vk_utils.h"
#include "common_graphics.h"

int gather_materials_for_opaque_pipeline (scene_asset_data* scene_data, vk_skeletal_opaque_graphics_pipeline* out_graphics_pipeline)
{
    OutputDebugString (L"gather_materials_for_opaque_pipeline\n");

    for (size_t m = 0; m < scene_data->materials_count; ++m)
    {
        vk_skeletal_material* material = scene_data->materials + m;

        if (strstr (material->name, "opaque") != NULL)
        {
            if (out_graphics_pipeline->materials == NULL)
            {
                out_graphics_pipeline->materials = (vk_skeletal_material**)utils_my_calloc (1, sizeof (vk_skeletal_material*));
            }
            else
            {
                out_graphics_pipeline->materials = (vk_skeletal_material**)utils_my_realloc_zero (out_graphics_pipeline->materials, sizeof (vk_skeletal_material*) * out_graphics_pipeline->materials_count, sizeof (vk_skeletal_material*) * (out_graphics_pipeline->materials_count + 1));
            }

            out_graphics_pipeline->materials[out_graphics_pipeline->materials_count] = material;

            ++out_graphics_pipeline->materials_count;
        }
    }

    return 0;
}

int create_opaque_graphics_pipeline (scene_asset_data* scene_data, vk_skeletal_opaque_graphics_pipeline** out_graphics_pipeline)
{
    OutputDebugString (L"create_opaque_graphics_pipeline\n");

    *out_graphics_pipeline = (vk_skeletal_opaque_graphics_pipeline*)utils_my_calloc (1, sizeof (vk_skeletal_opaque_graphics_pipeline));
    vk_skeletal_opaque_graphics_pipeline* graphics_pipeline = *out_graphics_pipeline;

    AGAINSTRESULT result;
    CHECK_AGAINST_RESULT (gather_materials_for_opaque_pipeline (scene_data, *out_graphics_pipeline), result);
    
    char full_vertex_shader_path[MAX_PATH];
    utils_get_full_file_path ("pbr_opaque.vert.spv", full_vertex_shader_path);
    
    char full_fragment_shader_path[MAX_PATH];
    utils_get_full_file_path ("pbr_opaque.frag.spv", full_fragment_shader_path);

    CHECK_AGAINST_RESULT (vk_utils_create_shader (full_vertex_shader_path, graphics_device, VK_SHADER_STAGE_VERTEX_BIT, &graphics_pipeline->shader_modules[0], &graphics_pipeline->shader_stage_create_infos[0]), result);
    CHECK_AGAINST_RESULT (vk_utils_create_shader (full_fragment_shader_path, graphics_device, VK_SHADER_STAGE_FRAGMENT_BIT, &graphics_pipeline->shader_modules[1], &graphics_pipeline->shader_stage_create_infos[1]), result);

    VkPushConstantRange push_constant_range = { 0 };
    push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.size = sizeof (int);

    return 0;
}

int gather_materials_for_alpha_pipeline (scene_asset_data* scene_data, vk_skeletal_alpha_graphics_pipeline* out_graphics_pipeline)
{
    OutputDebugString (L"gather_materials_for_alpha_pipeline\n");

    for (size_t m = 0; m < scene_data->materials_count; ++m)
    {
        vk_skeletal_material* material = scene_data->materials + m;

        if (strstr (material->name, "alpha") != NULL)
        {
            if (out_graphics_pipeline->materials == NULL)
            {
                out_graphics_pipeline->materials = (vk_skeletal_material**)utils_my_calloc (1, sizeof (vk_skeletal_material*));
            }
            else
            {
                out_graphics_pipeline->materials = (vk_skeletal_material**)utils_my_realloc_zero (out_graphics_pipeline->materials, sizeof (vk_skeletal_material*) * out_graphics_pipeline->materials_count, sizeof (vk_skeletal_material*) * (out_graphics_pipeline->materials_count + 1));
            }

            out_graphics_pipeline->materials[out_graphics_pipeline->materials_count] = material;

            ++out_graphics_pipeline->materials_count;
        }
    }

    return 0;
}

int create_alpha_graphics_pipeline (scene_asset_data* scene_data, vk_skeletal_alpha_graphics_pipeline** out_graphics_pipeline)
{
    OutputDebugString (L"create_alpha_graphics_pipeline\n");

    *out_graphics_pipeline = (vk_skeletal_alpha_graphics_pipeline*)utils_my_calloc (1, sizeof (vk_skeletal_alpha_graphics_pipeline));

    AGAINSTRESULT result;
    CHECK_AGAINST_RESULT (gather_materials_for_alpha_pipeline (scene_data, *out_graphics_pipeline), result);

    return 0;
}

void destroy_opaque_graphics_pipeline (vk_skeletal_opaque_graphics_pipeline* graphics_pipeline)
{
    OutputDebugString (L"destroy_opaque_graphics_pipeline\n");

    vkDestroyShaderModule (graphics_device, graphics_pipeline->shader_modules[0], NULL);
    vkDestroyShaderModule (graphics_device, graphics_pipeline->shader_modules[1], NULL);

    utils_my_free (graphics_pipeline->materials);
    utils_my_free (graphics_pipeline);
}

void destroy_alpha_graphics_pipeline (vk_skeletal_alpha_graphics_pipeline* graphics_pipeline)
{
    OutputDebugString (L"destroy_alpha_graphics_pipeline\n");

    utils_my_free (graphics_pipeline->materials);
    utils_my_free (graphics_pipeline);
}