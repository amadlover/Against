#pragma once

#include <stddef.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

typedef struct
{
    char name[1024];
    VkImage* image;
    VkImageView* image_view;
} vk_image;

typedef enum 
{
    opaque,
    mask,
    blend
} vk_material_alpha_mode;

typedef struct
{
    char name[1024];

    vk_image base_texture;
    vk_image metalness_roughness_texture;
    vk_image normal_texture;
    vk_image occlusion_texture;
    vk_image emissive_texture;

    float base_color_factor[4];
    float metalness_factor;
    float roughness_factor;
    float emissive_factor[3];

    vk_material_alpha_mode alpha_mode;
} vk_material;

typedef struct
{
    char name[1024];
} vk_animation;

typedef struct
{
    char name[1024];
} vk_skin;

typedef struct
{
    char name[1024];
} vk_static_mesh;

typedef struct
{
    char name[1024];
} vk_skeletal_mesh;