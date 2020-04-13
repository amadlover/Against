#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

#include "graphics_pipeline.h"

/*typedef struct
{
	char name[256];

	VkDeviceSize width;
	VkDeviceSize height;
	VkDeviceSize bpp;

	uint8_t* pixels;

	VkDeviceSize size;
	VkDeviceSize offset;

	VkImage* vk_image;
	uint32_t layer_index;
	VkImageView* image_view;
} asset_image;

typedef struct
{
	char name[256];
	asset_image image;
} asset_mesh_texture;

typedef struct
{
	char name[256];
	asset_mesh_texture base_color_texture;
} asset_mesh_material;

typedef struct
{
	uint8_t* positions;
	uint8_t* uv0s;
	uint8_t* uv1s;
	uint8_t* normals;
	uint8_t* indices;

	VkDeviceSize positions_size;
	VkDeviceSize uv0s_size;
	VkDeviceSize uv1s_size;
	VkDeviceSize normals_size;
	VkDeviceSize indices_size;

	VkBuffer buffer;
	VkDeviceMemory buffer_memory;

	uint32_t index_count;
	VkIndexType index_type;

	VkDeviceSize positions_offset;
	VkDeviceSize uv0s_offset;
	VkDeviceSize uv1s_offset;
	VkDeviceSize normals_offset;
	VkDeviceSize indices_offset;

	asset_mesh_material material;
} asset_mesh_graphics_primitive;

typedef struct
{
	uint8_t* positions;
	uint8_t* indices;

	uint32_t index_count;
	VkIndexType index_type;

	VkDeviceSize positions_size;
	VkDeviceSize indices_size;
} asset_mesh_physics_primitive;

typedef struct
{
	char name[256];
	uint32_t ID;

	asset_mesh_physics_primitive* physics_primitives;
	asset_mesh_graphics_primitive* graphics_primitives;

	uint32_t physics_primitive_count;
	uint32_t graphics_primitive_count;
} asset_mesh;

int import_asset_meshes (const char* file_path, asset_mesh** assets, uint32_t* asset_count);
void destroy_asset_meshes (asset_mesh* meshes, uint32_t mesh_count);
*/

typedef struct
{
	size_t x;
} asset_image;

typedef struct
{
	size_t x;
} asset_animation;

typedef struct
{
	size_t x;
} asset_skin;

typedef struct
{
	size_t x;
} asset_material_opaque;

typedef struct
{
	size_t x;
} asset_material_alpha;

typedef struct
{
	size_t x;
} asset_physics_primitive;

typedef struct
{
	size_t x;
} asset_graphics_primitive;

typedef struct
{
	char name[256];

	asset_graphics_primitive* graphics_primitives;
	size_t graphics_primitives_count;

	asset_physics_primitive* physics_primitives;
	size_t physics_primitives_count;
} asset_mesh;