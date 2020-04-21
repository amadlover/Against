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


typedef struct
{
    char name[1024];

} vk_animation;

typedef struct
{
    char name[1024];
    struct vk_joint* parent;

    float position[3];
    float rotation[4];
    float scale[3];
} vk_joint;

typedef struct
{
    char name[1024];

    size_t num_joints;
    size_t num_weights;
} vk_skin;

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
} vk_skeletal_material;

typedef struct
{
    VkBuffer* vb_ib;
    VkDeviceMemory* vb_ib_memory;

    size_t positions_offset;
    size_t normals_offset;
    size_t uv0s_offset;
    size_t uv1s_offset;
    size_t weights_offset;
    size_t joints_offset;
    size_t indices_offset;

    size_t indices_count;

    VkIndexType index_type;

    vk_skeletal_material* material;
} vk_skeletal_graphics_primitive;

typedef struct
{
    char name[1024];
    
    vk_skeletal_graphics_primitive** graphics_primitives;
    size_t graphics_primtives_count;
} vk_skeletal_mesh;