#include "graphics_pipeline.h"
#include "utils.h"
#include "vk_utils.h"
#include "common_graphics.h"

AGAINST_RESULT create_skybox_graphics_pipeline (scene_graphics* scene, vk_skybox_graphics_pipeline** out_graphics_pipeline)
{
    OutputDebugString (L"create_skybox_graphics_pipeine\n");

    return AGAINST_SUCCESS;
}

AGAINST_RESULT create_skinned_opaque_graphics_pipeline (scene_graphics* scene, vk_skinned_opaque_graphics_pipeline** out_graphics_pipeline)
{
    OutputDebugString (L"create_skinned_opaque_graphics_pipeline\n");

    *out_graphics_pipeline = (vk_skinned_opaque_graphics_pipeline*)utils_calloc (1, sizeof (vk_skinned_opaque_graphics_pipeline));
    vk_skinned_opaque_graphics_pipeline* graphics_pipeline = *out_graphics_pipeline;

    AGAINST_RESULT result = AGAINST_SUCCESS;
    
    char full_vertex_shader_path[MAX_PATH];
    utils_get_full_file_path ("pbr_opaque.vert.spv", full_vertex_shader_path);
    
    char full_fragment_shader_path[MAX_PATH];
    utils_get_full_file_path ("pbr_opaque.frag.spv", full_fragment_shader_path);

    CHECK_AGAINST_RESULT (vk_utils_create_shader (full_vertex_shader_path, graphics_device, VK_SHADER_STAGE_VERTEX_BIT, &graphics_pipeline->shader_modules[0], &graphics_pipeline->shader_stage_create_infos[0]), result);
    CHECK_AGAINST_RESULT (vk_utils_create_shader (full_fragment_shader_path, graphics_device, VK_SHADER_STAGE_FRAGMENT_BIT, &graphics_pipeline->shader_modules[1], &graphics_pipeline->shader_stage_create_infos[1]), result);

    VkPushConstantRange push_constant_range = { 0 };
    push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.size = sizeof (int);

    VkDescriptorType descriptor_types[2] = { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER };
    size_t descriptor_types_count[2] = { 1, 1 };
    CHECK_AGAINST_RESULT (vk_utils_create_descriptor_pool (graphics_device, descriptor_types, descriptor_types_count, 2, 2, &scene->descriptor_pool), result);
    
    VkDescriptorSetLayoutBinding descriptor_set_layout_bindings[2] = { 0, 0 };
    size_t descriptor_count_per_type[2] = { 1, 5 };
    size_t bindings[2] = { 0, 0 };
    VkShaderStageFlags shader_stage_flags[2] = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
    CHECK_AGAINST_RESULT (vk_utils_create_descriptor_set_layout_bindings (graphics_device, descriptor_types, descriptor_count_per_type, bindings, 2, shader_stage_flags, descriptor_set_layout_bindings), result);

    CHECK_AGAINST_RESULT (vk_utils_create_descriptor_set_layout (graphics_device, descriptor_set_layout_bindings, 2, &graphics_pipeline->descriptor_set_layout), result);

    return AGAINST_SUCCESS;
}

void destroy_skinned_opaque_graphics_pipeline (vk_skinned_opaque_graphics_pipeline* graphics_pipeline)
{
    OutputDebugString (L"destroy_skinned_opaque_graphics_pipeline\n");

    vkDestroyShaderModule (graphics_device, graphics_pipeline->shader_modules[0], NULL);
    vkDestroyShaderModule (graphics_device, graphics_pipeline->shader_modules[1], NULL);

    vkDestroyDescriptorSetLayout (graphics_device, graphics_pipeline->descriptor_set_layout, NULL);

    utils_free (graphics_pipeline);
}
