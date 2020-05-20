#pragma once

#include <stddef.h>
#include <stdint.h>

#include <vulkan/vulkan.h>

#define MAX_JOINTS 100

typedef struct _vk_image
{
    char name[1024];

    VkImage* image;
    VkImageView* image_view;
} vk_image;

typedef struct _vk_animation
{
    char name[1024];

    VkDeviceSize* frame_data_offsets;
    size_t num_frames;
} vk_animation;

typedef enum _vk_material_alpha_mode
{
    opaque,
    mask,
    blend
} vk_material_alpha_mode;

typedef struct _vk_material
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

typedef struct _vk_skinned_primitive
{
    size_t positions_offset;
    size_t normals_offset;
    size_t uv0s_offset;
    size_t uv1s_offset;
    size_t weights_offset;
    size_t joints_offset;
    size_t indices_offset;

    size_t num_indices;

    VkIndexType index_type;
    
    vk_material* material;
} vk_skinned_primitive;

typedef struct _vk_static_primitive
{
    size_t positions_offset;
    size_t normals_offset;
    size_t uv0s_offset;
    size_t uv1s_offset;
    size_t indices_offset;

    size_t num_indices;
    VkIndexType index_type;

    vk_material* material;
} vk_static_primitive;

typedef struct _vk_skinned_mesh
{
    char name[1024];

    vk_skinned_primitive** opaque_graphics_primitives;
    size_t num_opaque_graphics_primitives;

    vk_skinned_primitive** alpha_graphics_primitives;
    size_t num_alpha_graphics_primitives;

    vk_skinned_primitive** blend_graphics_primitives;
    size_t num_blend_graphics_primitives;
} vk_skinned_mesh;

typedef struct _vk_static_mesh
{
    char name[1024];

    vk_static_primitive** opaque_graphics_primitives;
    size_t num_opaque_graphics_primitives;

    vk_static_primitive** alpha_graphics_primitives;
    size_t num_alpha_graphics_primitives;

    vk_static_primitive** blend_graphics_primitives;
    size_t num_blend_graphics_primitives;
} vk_static_mesh;

typedef struct _vk_skin
{
    char name[1024];

    VkDeviceSize bind_pose_offset;
    
    vk_animation** animations;
    size_t num_animations;

    vk_skinned_mesh** skinned_meshes;
    size_t num_skinned_meshes;
} vk_skin;