#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

typedef struct
{
	char name[256];

	VkDeviceSize width;
	VkDeviceSize height;
	VkDeviceSize bpp;

	uint8_t* pixels;

	VkDeviceSize size;
	VkDeviceSize offset;

	VkImage* image;
	uint32_t layer_index;
	VkImageView* image_view;
} image;

typedef struct
{
	char name[256];
	image* image;
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

	asset_mesh_physics_primitive* asset_mesh_physics_primitive;
	asset_mesh_graphics_primitive* asset_mesh_graphics_primitive;

	uint32_t physics_primitive_count;
	uint32_t graphics_primitive_count;
} asset_mesh;

int import_asset_meshes (const char* file_path, asset_mesh** assets, uint32_t* asset_count, image* images);
void destroy_asset_meshes (asset_mesh* meshes, uint32_t mesh_count);

int import_images (const char* file_path, image** images, uint32_t* image_count);