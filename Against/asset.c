#include "Asset.h"
#include "Error.h"

#include "graphics_utils.h"
#include "utils.h"

#include <Windows.h>
#include <Shlwapi.h>
#include <strsafe.h>

#include <string.h>

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void import_graphics_primitive_attribute (asset_mesh_graphics_primitive* graphics_primitive, const cgltf_attribute* attribute)
{
	cgltf_accessor* accessor = attribute->data;
	cgltf_buffer_view* buffer_view = accessor->buffer_view;

	if (attribute->type == cgltf_attribute_type_position)
	{
		uint8_t* positions = (uint8_t*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;

		graphics_primitive->positions_size = buffer_view->size;
		graphics_primitive->positions = (uint8_t*)my_malloc (buffer_view->size);
		memcpy (graphics_primitive->positions, positions, buffer_view->size);
	}
	else if (attribute->type == cgltf_attribute_type_texcoord)
	{
		if (strcmp (attribute->name, "TEXCOORD_0") == 0)
		{
			uint8_t* uv0s = (uint8_t*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;

			graphics_primitive->uv0s_size = buffer_view->size;
			graphics_primitive->uv0s = (uint8_t*)my_malloc (buffer_view->size);
			memcpy (graphics_primitive->uv0s, uv0s, buffer_view->size);
		}
		else if (strcmp (attribute->name, "TEXCOORD_1") == 0)
		{
			uint8_t* uv1s = (uint8_t*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;

			graphics_primitive->uv1s_size = buffer_view->size;
			graphics_primitive->uv1s = (uint8_t*)my_malloc (buffer_view->size);
			memcpy (graphics_primitive->uv1s, uv1s, buffer_view->size);
		}
	}
	else if (attribute->type == cgltf_attribute_type_normal)
	{
		uint8_t* normals = (uint8_t*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;

		graphics_primitive->normals_size = buffer_view->size;
		graphics_primitive->normals = (uint8_t*)my_malloc (buffer_view->size);
		memcpy (graphics_primitive->normals, normals, buffer_view->size);
	}
}

void import_graphics_primitive_material (const char* file_path, asset_mesh_graphics_primitive* graphics_primitive, const cgltf_material* material, const cgltf_data* data)
{
	strcpy (graphics_primitive->material.name, material->name);

	if (material->has_pbr_metallic_roughness)
	{
		if (material->pbr_metallic_roughness.base_color_texture.texture->name != NULL)
		{
			strcpy (graphics_primitive->material.base_color_texture.name, material->pbr_metallic_roughness.base_color_texture.texture->name);
		}

		char full_texture_path[256];
		get_full_texture_path_from_uri (file_path, material->pbr_metallic_roughness.base_color_texture.texture->image->uri, full_texture_path);
		graphics_primitive->material.base_color_texture.image.pixels = stbi_load (full_texture_path, (int*)&graphics_primitive->material.base_color_texture.image.width, (int*)&graphics_primitive->material.base_color_texture.image.height, (int*)&graphics_primitive->material.base_color_texture.image.bpp, 4);
		graphics_primitive->material.base_color_texture.image.size = graphics_primitive->material.base_color_texture.image.width * graphics_primitive->material.base_color_texture.image.height * graphics_primitive->material.base_color_texture.image.bpp;
	}
}

void import_graphics_primitive_indices (asset_mesh_graphics_primitive* graphics_primitive, const cgltf_primitive* primitive)
{
	cgltf_accessor* accessor = primitive->indices;
	cgltf_buffer_view* buffer_view = accessor->buffer_view;

	char* indices = (char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
	graphics_primitive->index_count = (uint32_t)accessor->count;
	graphics_primitive->indices_size = (VkDeviceSize)buffer_view->size;
	graphics_primitive->indices = (uint8_t*)my_malloc (buffer_view->size);
	memcpy (graphics_primitive->indices, indices, buffer_view->size);
	
	switch (accessor->component_type)
	{
	case cgltf_component_type_r_16u:
		graphics_primitive->index_type = VK_INDEX_TYPE_UINT16;
		break;

	case cgltf_component_type_r_32u:
		graphics_primitive->index_type = VK_INDEX_TYPE_UINT32;
		break;

	default:
		break;
	}
}

void import_physics_primitive_indices (asset_mesh_physics_primitive* physics_primitive, const cgltf_primitive* primitive)
{
	cgltf_accessor* accessor = primitive->indices;
	cgltf_buffer_view* buffer_view = accessor->buffer_view;

	char* indices = (char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
	physics_primitive->index_count = (uint32_t)accessor->count;
	physics_primitive->indices_size = (VkDeviceSize)buffer_view->size;
	physics_primitive->indices = (uint8_t*)my_malloc (buffer_view->size);
	memcpy (physics_primitive->indices, indices, buffer_view->size);

	switch (accessor->component_type)
	{
	case cgltf_component_type_r_16u:
		physics_primitive->index_type = VK_INDEX_TYPE_UINT16;
		break;

	case cgltf_component_type_r_32u:
		physics_primitive->index_type = VK_INDEX_TYPE_UINT32;
		break;

	default:
		break;
	}
}

int import_mesh_graphics_primitives (const char* file_path, asset_mesh* meshes, const cgltf_data* data)
{
	uint32_t graphics_mesh_counter = 0;

	for (uint32_t n = 0; n < data->nodes_count; n++)
	{
		cgltf_node* node = data->nodes + n;

		if (node->mesh == NULL)
		{
			continue;
		}

		if (strstr (node->name, "CS_") != NULL)
		{
			continue;
		}

		asset_mesh* current_mesh = meshes + graphics_mesh_counter;
		strcpy (current_mesh->name, node->name);

		current_mesh->graphics_primitive_count = (uint32_t)node->mesh->primitives_count;
		current_mesh->graphics_primitives = (asset_mesh_graphics_primitive*)my_calloc (current_mesh->graphics_primitive_count, sizeof (asset_mesh_graphics_primitive));

		for (uint32_t p = 0; p < node->mesh->primitives_count; p++)
		{
			cgltf_primitive* primitive = node->mesh->primitives + p;
			asset_mesh_graphics_primitive* current_graphics_primitive = current_mesh->graphics_primitives + p;

			for (uint32_t a = 0; a < primitive->attributes_count; a++)
			{
				import_graphics_primitive_attribute (current_graphics_primitive, primitive->attributes + a);
			}

			import_graphics_primitive_material (file_path, current_graphics_primitive, primitive->material, data);
			import_graphics_primitive_indices (current_graphics_primitive, primitive);
		}

		++graphics_mesh_counter;
	}

	return 0;
}

int import_mesh_physics_primitives (const char* file_path, asset_mesh* meshes, const cgltf_data* data)
{
	return 0;
}

int import_asset_meshes (const char* file_path, asset_mesh** meshes, uint32_t* mesh_count)
{
	cgltf_options options = { 0 };
	cgltf_data* data = NULL;

	cgltf_result result = cgltf_parse_file (&options, file_path, &data);

	if (result == cgltf_result_success)
	{
		result = cgltf_load_buffers (&options, data, file_path);

		if (result == cgltf_result_success)
		{
			result = cgltf_validate (data);

			if (result == cgltf_result_success)
			{
				for (uint32_t n = 0; n < data->nodes_count; n++)
				{
					cgltf_node* node = data->nodes + n;

					if (node->mesh == NULL) continue;

					if (strstr (node->name, "CS_") != NULL) continue;

					++(*mesh_count);
				}

				*meshes = (asset_mesh*)my_calloc ((size_t)(*mesh_count), sizeof (asset_mesh));

				import_mesh_graphics_primitives (file_path, *meshes, data);
				import_mesh_physics_primitives (file_path, *meshes, data);
			}
			else
			{
				return AGAINST_ERROR_GLTF_IMPORT;
			}
		}
		else
		{
			return AGAINST_ERROR_GLTF_IMPORT;
		}
	}
	else 
	{
		return AGAINST_ERROR_GLTF_IMPORT;
	}

	cgltf_free (data);

	return 0;
}

void destroy_asset_meshes (asset_mesh* assets, uint32_t asset_count)
{
	for (uint32_t a = 0; a < asset_count; a++)
	{
		asset_mesh* current_asset = assets + a;

		if (current_asset->graphics_primitives)
		{
			for (uint32_t i = 0; i < current_asset->graphics_primitive_count; i++)
			{
				asset_mesh_graphics_primitive* current_gp = current_asset->graphics_primitives + i;

				if (current_gp->indices)
				{
					my_free (current_gp->indices);
				}

				if (current_gp->positions)
				{
					my_free (current_gp->positions);
				}

				if (current_gp->uv0s)
				{
					my_free (current_gp->uv0s);
				}

				if (current_gp->uv1s)
				{
					my_free (current_gp->uv1s);
				}

				if (current_gp->normals)
				{
					my_free (current_gp->normals);
				}

				if (current_gp->material.base_color_texture.image.pixels)
				{
					my_free (current_gp->material.base_color_texture.image.pixels);
				}
			}

			my_free (current_asset->graphics_primitives);
		}

		if (current_asset->physics_primitives)
		{
			for (uint32_t i = 0; i < current_asset->physics_primitive_count; i++)
			{
				asset_mesh_physics_primitive* current_pp = current_asset->physics_primitives + i;

				if (current_pp->indices)
				{
					my_free (current_pp->indices);
				}

				if (current_pp->positions)
				{
					my_free (current_pp->positions);
				}
			}

			my_free (current_asset->physics_primitives);
		}
	}

	my_free (assets);
}

int import_images (const char* file_path, image** images, uint32_t* image_count)
{
	cgltf_options options = { 0 };
	cgltf_data* data = NULL;

	cgltf_result result = cgltf_parse_file (&options, file_path, &data);

	if (result == cgltf_result_success)
	{
		result = cgltf_load_buffers (&options, data, file_path);

		if (result == cgltf_result_success)
		{
			result = cgltf_validate (data);

			if (result == cgltf_result_success)
			{
				*image_count = (uint32_t)data->images_count;
				*images = (image*)my_calloc (data->images_count, sizeof (image));

				for (uint32_t i = 0; i < data->images_count; i++)
				{
					cgltf_image* I = data->images + i;
					image* current_image = (*images) + i;

					strcpy (current_image->name, I->name);

					char full_texture_path[256];

					get_full_texture_path_from_uri (file_path, I->uri, full_texture_path);
					current_image->pixels = stbi_load (full_texture_path, (int*)&current_image->width, (int*)&current_image->height, (int*)&current_image->bpp, 4);
					current_image->size = current_image->width * current_image->height * current_image->bpp;
				}
			}
		}
	}

	cgltf_free (data);

	return 0;
}