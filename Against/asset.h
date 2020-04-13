#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct
{
    char name[1024];

    size_t width;
    size_t height;
    size_t bpp;

    uint8_t* pixels;

    size_t size;
} asset_image;

typedef struct
{
    size_t x;
} asset_sampler;

typedef struct
{
    char name[1024];
    asset_image* image;
} asset_texture;

typedef enum 
{
    opaque,
    mask,
    blend
} asset_material_alpha_mode;

typedef struct
{
    char name[1024];

    asset_texture* base_texture;
    asset_texture* metalness_roughness_texture;
    asset_texture* normal_texture;
    asset_texture* occlusion_texture;
    asset_texture* emissive_texture;

    float base_color_factor[4];
    float metalness_factor;
    float roughness_factor;
    float emissive_factor[4];

    asset_material_alpha_mode alpha_mode;
} asset_material;

typedef struct
{
    char name[1024];
} asset_animation;

typedef struct
{
    char name[1024];
} asset_skin;

typedef struct
{
    char name[1024];
} asset_static_mesh;

typedef struct
{
    char name[1024];
} asset_skeletal_mesh;