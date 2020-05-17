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
    size_t frames_count;
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

typedef struct _vk_skeletal_graphics_primitive
{
    size_t positions_offset;
    size_t normals_offset;
    size_t uv0s_offset;
    size_t uv1s_offset;
    size_t weights_offset;
    size_t joints_offset;
    size_t indices_offset;

    size_t indices_count;

    VkIndexType index_type;
    
    vk_material* material;
} vk_skeletal_graphics_primitive;

typedef struct _vk_skeletal_physics_primitive
{
    size_t positions_offset;
    size_t weights_offset;
    size_t joints_offset;
    size_t indices_offset;

    size_t indices_count;
    VkIndexType index_type;
} vk_skeletal_physics_primitive;

typedef struct _vk_static_graphics_primitive
{
    size_t positions_offset;
    size_t normals_offset;
    size_t uv0s_offset;
    size_t uv1s_offset;
    size_t indices_offset;

    size_t indices_count;
    VkIndexType index_type;

    vk_material* material;
} vk_static_graphics_primitive;

typedef struct _vk_static_physics_primitive
{
    size_t positions_offset;
    size_t indices_offset;

    size_t indices_count;
    VkIndexType index_type;
} vk_static_physics_primitive;

typedef struct _vk_skeletal_mesh
{
    char name[1024];

    vk_skeletal_graphics_primitive** opaque_graphics_primitives;
    size_t opaque_graphics_primitives_count;

    vk_skeletal_graphics_primitive** alpha_graphics_primitives;
    size_t alpha_graphics_primitives_count;

    vk_skeletal_graphics_primitive** blend_graphics_primitives;
    size_t blend_graphics_primitives_count;

    vk_skeletal_physics_primitive** physics_primitives;
    size_t physics_primitives_count;
} vk_skeletal_mesh;

typedef struct _vk_static_mesh
{
    char name[1024];

    vk_static_graphics_primitive** opaque_graphics_primitives;
    size_t opaque_graphics_primitive_count;

    vk_static_graphics_primitive** alpha_graphics_primitives;
    size_t alpha_graphics_primitive_count;

    vk_static_graphics_primitive** blend_graphics_primitives;
    size_t blend_graphics_primitive_count;

    vk_static_physics_primitive** physics_primitives;
    size_t physics_primitives_count;
} vk_static_mesh;

typedef struct _vk_skin
{
    char name[1024];

    VkDeviceSize bind_pose_offset;
    
    vk_animation** animations;
    size_t animations_count;

    vk_skeletal_mesh** skeletal_meshes;
    size_t skeletal_meshes_count;
} vk_skin;