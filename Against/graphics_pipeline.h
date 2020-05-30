#pragma once

#include "error.h"
#include "scene.h"

typedef struct _vk_skinned_opaque_graphics_pipeline
{
    VkShaderModule shader_modules[2];
    VkPipelineShaderStageCreateInfo shader_stage_create_infos[2];
    VkDescriptorSetLayout descriptor_set_layout;
} vk_skinned_opaque_graphics_pipeline;

typedef struct _vk_skybox_graphics_pipeline
{
    VkShaderModule shader_modules[2];
    VkPipelineShaderStageCreateInfo shader_stage_create_infos[2];
    VkDescriptorSetLayout descriptor_set_layout;
} vk_skybox_graphics_pipeline;

AGAINST_RESULT create_skybox_graphics_pipeline (scene_graphics* scene, vk_skybox_graphics_pipeline** out_graphics_pipeline);
AGAINST_RESULT create_skinned_opaque_graphics_pipeline (scene_graphics* scene, vk_skinned_opaque_graphics_pipeline** out_graphics_pipeline);

void destroy_skinned_opaque_graphics_pipeline (vk_skinned_opaque_graphics_pipeline* graphics_pipeline); 
