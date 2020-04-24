#include "gltf.h"
#include "utils.h"
#include "error.h"
#include "common_graphics.h"
#include "vk_utils.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <stb_image.h>

cgltf_data** gltf_datas = NULL;
size_t num_gltf_datas = 0;

cgltf_image** ref_cgltf_images_for_materials = NULL;
size_t num_ref_cgltf_images_for_materials = 0;

cgltf_material** ref_cgltf_materials_for_linking_graphics_primitives = NULL;
size_t num_ref_cgltf_materials_for_linking_graphics_primitives = 0;

cgltf_animation** ref_cgltf_anims_for_meshes = NULL;
size_t num_ref_cgltf_anims_for_meshes = 0;

cgltf_primitive** ref_cgltf_graphics_primitives_for_linking_materials = NULL;
size_t num_ref_cgltf_graphics_primitives_for_linking_materials = 0;


int import_images (const char* full_folder_path, cgltf_data** datas, size_t num_datas, scene_asset_data* out_data)
{
    OutputDebugString (L"import_images\n");

    size_t total_image_size = 0;

    size_t* img_offsets = NULL;
    size_t* img_sizes = NULL;
    size_t* img_widths = NULL;
    size_t* img_heights = NULL;
    uint8_t** img_pixels = NULL;

    AGAINSTRESULT result;
    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        if (out_data->images == NULL)
        {
            out_data->images = (VkImage*)utils_my_calloc (current_data->images_count, sizeof (VkImage));
        }
        else
        {
            out_data->images = (VkImage*)utils_my_realloc_zero (out_data->images, sizeof (VkImage) * out_data->images_count, sizeof (VkImage) * (out_data->images_count + current_data->images_count));
        }

        if (out_data->image_views == NULL)
        {
            out_data->image_views = (VkImageView*)utils_my_calloc (current_data->images_count, sizeof (VkImageView));
        }
        else
        {
            out_data->image_views = (VkImageView*)utils_my_realloc_zero (out_data->image_views, sizeof (VkImageView) * out_data->images_count, sizeof (VkImageView) * (out_data->images_count + current_data->images_count));
        }

        if (img_offsets == NULL)
        {
            img_offsets = (size_t*)utils_my_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_offsets = (size_t*)utils_my_realloc_zero (img_offsets, sizeof (size_t) * out_data->images_count, sizeof (size_t) * (out_data->images_count + current_data->images_count));
        }

        if (img_sizes == NULL)
        {
            img_sizes = (size_t*)utils_my_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_sizes = (size_t*)utils_my_realloc_zero (img_sizes, sizeof (size_t) * out_data->images_count, sizeof (size_t) * (out_data->images_count + current_data->images_count));
        }

        if (img_widths == NULL)
        {
            img_widths = (size_t*)utils_my_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_widths = (size_t*)utils_my_realloc_zero (img_widths, sizeof (size_t) * out_data->images_count, sizeof (size_t) * (out_data->images_count + current_data->images_count));
        }

        if (img_heights == NULL)
        {
            img_heights = (size_t*)utils_my_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_heights = (size_t*)utils_my_realloc_zero (img_heights, sizeof (size_t) * out_data->images_count, sizeof (size_t) * (out_data->images_count + current_data->images_count));
        }

        if (img_pixels == NULL)
        {
            img_pixels = (uint8_t**)utils_my_calloc (current_data->images_count, sizeof (uint8_t*));
        }
        else
        {
            img_pixels = (uint8_t**)utils_my_realloc_zero (img_pixels, sizeof (uint8_t*) * out_data->images_count, sizeof (uint8_t*) * (out_data->images_count + current_data->images_count));
        }

        if (ref_cgltf_images_for_materials == NULL)
        {
            ref_cgltf_images_for_materials = (cgltf_image**)utils_my_calloc (current_data->images_count, sizeof (cgltf_image*));
        }
        else
        {
            ref_cgltf_images_for_materials = (cgltf_image**)utils_my_realloc_zero (ref_cgltf_images_for_materials, sizeof (cgltf_image*) * out_data->images_count , sizeof (cgltf_image*) * (out_data->images_count + current_data->images_count));
        }

        for (size_t i = 0; i < current_data->images_count; ++i)
        {
            size_t current_index = num_ref_cgltf_images_for_materials + i;

            cgltf_image* image = current_data->images + i;
            char full_texture_path[MAX_PATH];
            utils_get_full_texture_path_from_uri (full_folder_path, image->uri, full_texture_path);
            int bpp;
            img_pixels[current_index] = stbi_load (full_texture_path, img_widths + current_index, img_heights + current_index, &bpp, 4);

            VkExtent3D image_extent = { img_widths[current_index], img_heights[current_index], 1 };
            CHECK_AGAINST_RESULT (vk_utils_create_image (graphics_device, graphics_queue_family_index, image_extent, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_SHARING_MODE_EXCLUSIVE, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, &out_data->images[current_index]), result);
            img_sizes[current_index] = img_widths[current_index] * img_heights[current_index] * 4;
            total_image_size += img_sizes[current_index];
            img_offsets[current_index] = current_index > 0 ? total_image_size - img_sizes[current_index] : 0;

            ref_cgltf_images_for_materials[current_index] = image;
        }

        num_ref_cgltf_images_for_materials += current_data->images_count;
        out_data->images_count += current_data->images_count;
    }

    VkBuffer staging_buffer; VkDeviceMemory staging_memory;
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (graphics_device, total_image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &staging_buffer), result);
    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (graphics_device, &staging_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_memory), result);

    for (size_t i = 0; i < num_ref_cgltf_images_for_materials; ++i)
    {
        CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (graphics_device, staging_memory, img_offsets[i], img_sizes[i], img_pixels[i]), result);
    }

    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_image_memory (graphics_device, out_data->images, num_ref_cgltf_images_for_materials, physical_device_memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &out_data->images_memory), result);

    for (size_t i = 0; i < num_ref_cgltf_images_for_materials; ++i)
    {
        VkExtent3D img_extent = { img_widths[i], img_heights[i], 1 };
        CHECK_AGAINST_RESULT (vk_utils_change_image_layout (graphics_device, graphics_queue, common_command_pool, graphics_queue_family_index, out_data->images[i], 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT), result);
        CHECK_AGAINST_RESULT (vk_utils_copy_buffer_to_image (graphics_device, common_command_pool, graphics_queue, img_offsets[i], staging_buffer, &out_data->images[i], img_extent, 1), result);
        CHECK_AGAINST_RESULT (vk_utils_change_image_layout (graphics_device, graphics_queue, common_command_pool, graphics_queue_family_index, out_data->images[i], 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT), result);
        CHECK_AGAINST_RESULT (vk_utils_create_image_view (graphics_device, out_data->images[i], &out_data->image_views[i]), result);
    }

    utils_my_free (img_offsets);
    utils_my_free (img_sizes);
    utils_my_free (img_widths);
    utils_my_free (img_heights);
    
    for (size_t i = 0; i < num_ref_cgltf_images_for_materials; ++i)
    {
        stbi_image_free (img_pixels[i]);
    }
    
    utils_my_free (img_pixels);

    vk_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_memory);

    return 0;
}


int import_materials (cgltf_data** datas, size_t num_datas, scene_asset_data* out_data)
{
    OutputDebugString (L"import_materials\n");
    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        if (out_data->materials == NULL)
        {
            out_data->materials = (vk_skeletal_material*)utils_my_calloc (current_data->materials_count, sizeof (vk_skeletal_material));
        }
        else
        {
            out_data->materials = (vk_skeletal_material*)utils_my_realloc_zero (out_data->materials, sizeof (vk_skeletal_material) * out_data->materials_count, sizeof (vk_skeletal_material) * (current_data->materials_count + out_data->materials_count));
        }

        if (ref_cgltf_materials_for_linking_graphics_primitives == NULL)
        {
            ref_cgltf_materials_for_linking_graphics_primitives = (cgltf_material**)utils_my_calloc (current_data->materials_count, sizeof (cgltf_material*));
        }
        else
        {
            ref_cgltf_materials_for_linking_graphics_primitives = (cgltf_material**)utils_my_realloc_zero (ref_cgltf_materials_for_linking_graphics_primitives, sizeof (cgltf_material*) * out_data->materials_count, sizeof (cgltf_material*) * (out_data->materials_count + current_data->materials_count));
        }

        for (size_t m = 0; m < current_data->materials_count; ++m)
        {
            size_t current_index = out_data->materials_count + m;
            cgltf_material* material = current_data->materials + m;

            strcpy (out_data->materials[current_index].name, material->name);

            for (size_t i = 0; i < num_ref_cgltf_images_for_materials; ++i)
            {
                if (material->pbr_metallic_roughness.base_color_texture.texture)
                {
                    if (material->pbr_metallic_roughness.base_color_texture.texture->image == ref_cgltf_images_for_materials[i])
                    {
                        strcpy (out_data->materials[current_index].base_texture.name, ref_cgltf_images_for_materials[i]->name);
                        out_data->materials[current_index].base_texture.image = out_data->images + current_index;
                        out_data->materials[current_index].base_texture.image_view = out_data->image_views + current_index;
                    }
                }

                if (material->pbr_metallic_roughness.metallic_roughness_texture.texture)
                {
                    if (material->pbr_metallic_roughness.metallic_roughness_texture.texture->image == ref_cgltf_images_for_materials[i])
                    {
                        strcpy (out_data->materials[current_index].metalness_roughness_texture.name, ref_cgltf_images_for_materials[i]->name);
                        out_data->materials[current_index].metalness_roughness_texture.image = out_data->images + current_index;
                        out_data->materials[current_index].metalness_roughness_texture.image_view = out_data->image_views + current_index;
                    }
                }

                if (material->normal_texture.texture)
                {
                    if (material->normal_texture.texture->image == ref_cgltf_images_for_materials[i])
                    {
                        strcpy (out_data->materials[current_index].normal_texture.name, ref_cgltf_images_for_materials[i]->name);
                        out_data->materials[current_index].normal_texture.image = out_data->images + current_index;
                        out_data->materials[current_index].normal_texture.image_view = out_data->image_views + current_index;
                    }
                }

                if (material->emissive_texture.texture)
                {
                    if (material->emissive_texture.texture->image == ref_cgltf_images_for_materials[i])
                    {
                        strcpy (out_data->materials[current_index].emissive_texture.name, ref_cgltf_images_for_materials[i]->name);
                        out_data->materials[current_index].emissive_texture.image = out_data->images + current_index;
                        out_data->materials[current_index].emissive_texture.image_view = out_data->image_views + current_index;
                    }
                }

                if (material->occlusion_texture.texture)
                {
                    if (material->occlusion_texture.texture->image == ref_cgltf_images_for_materials[i])
                    {
                        strcpy (out_data->materials[current_index].occlusion_texture.name, ref_cgltf_images_for_materials[i]->name);
                        out_data->materials[current_index].occlusion_texture.image = out_data->images + current_index;
                        out_data->materials[current_index].occlusion_texture.image_view = out_data->image_views + current_index;
                    }
                }
            }

            out_data->materials[current_index].alpha_mode = material->alpha_mode;
            out_data->materials[current_index].metalness_factor = material->pbr_metallic_roughness.metallic_factor;
            out_data->materials[current_index].roughness_factor = material->pbr_metallic_roughness.roughness_factor;
            memcpy (out_data->materials[current_index].emissive_factor, material->emissive_factor, sizeof (float) * 4);
            memcpy (out_data->materials[current_index].base_color_factor, material->pbr_metallic_roughness.base_color_factor, sizeof (float) * 4);

            ref_cgltf_materials_for_linking_graphics_primitives[current_index] = material;
        }

        out_data->materials_count += current_data->materials_count;
        num_ref_cgltf_materials_for_linking_graphics_primitives += current_data->materials_count;
    }

    return 0;
}

int import_graphics_primitives (cgltf_data** datas, size_t num_datas, scene_asset_data* out_data)
{
    OutputDebugString (L"import_graphics_primitives\n");
    unsigned char** positions = NULL;
    unsigned char** normals = NULL;
    unsigned char** uv0s = NULL;
    unsigned char** uv1s = NULL;
    unsigned char** joints = NULL;
    unsigned char** weights = NULL;
    unsigned char** indices = NULL;

    size_t* positions_sizes = NULL;
    size_t* positions_offsets = NULL;
    size_t* normals_sizes = NULL;
    size_t* normals_offsets = NULL;
    size_t* uv0s_sizes = NULL;
    size_t* uv0s_offsets = NULL;
    size_t* uv1s_sizes = NULL;
    size_t* uv1s_offsets = NULL;
    size_t* joints_sizes = NULL;
    size_t* joints_offsets = NULL;
    size_t* weights_sizes = NULL;
    size_t* weights_offsets = NULL;
    size_t* indices_counts = NULL;
    size_t* indices_sizes = NULL;
    size_t* indices_offsets = NULL;
    VkIndexType* indices_types = NULL;

    size_t num_primitives_data = 0;
    
    size_t total_data_size = 0;
    size_t current_primitive_data_offset = 0;

    size_t current_primitive_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        for (size_t n = 0; n < current_data->nodes_count; ++n)
        {
            cgltf_node* current_node = current_data->nodes + n;

            if (current_node->mesh == NULL) continue;
            if (strstr (current_node->name, "CS_") != NULL) continue;

            cgltf_mesh* current_mesh = current_node->mesh;

            if (positions == NULL)
            {
                positions = (unsigned char**)utils_my_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                positions = (unsigned char**)utils_my_realloc (positions, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (positions_sizes == NULL)
            {
                positions_sizes = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                positions_sizes = (size_t*)utils_my_realloc (positions_sizes, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (positions_offsets == NULL)
            {
                positions_offsets = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                positions_offsets = (size_t*)utils_my_realloc (positions_offsets, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (normals == NULL)
            {
                normals = (unsigned char**)utils_my_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                normals = (unsigned char**)utils_my_realloc (normals, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (normals_sizes == NULL)
            {
                normals_sizes = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                normals_sizes = (size_t*)utils_my_realloc (normals_sizes, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (normals_offsets == NULL)
            {
                normals_offsets = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                normals_offsets = (size_t*)utils_my_realloc (normals_offsets, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv0s == NULL)
            {
                uv0s = (unsigned char**)utils_my_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                uv0s = (unsigned char**)utils_my_realloc (uv0s, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv0s_sizes == NULL)
            {
                uv0s_sizes = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                uv0s_sizes = (size_t*)utils_my_realloc (uv0s_sizes, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv0s_offsets == NULL)
            {
                uv0s_offsets = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                uv0s_offsets = (size_t*)utils_my_realloc (uv0s_offsets, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv1s == NULL)
            {
                uv1s = (unsigned char**)utils_my_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                uv1s = (unsigned char**)utils_my_realloc_zero (uv1s, sizeof (unsigned char*) * num_primitives_data , sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv1s_sizes == NULL)
            {
                uv1s_sizes = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                uv1s_sizes = (size_t*)utils_my_realloc_zero (uv1s_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv1s_offsets == NULL)
            {
                uv1s_offsets = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                uv1s_offsets = (size_t*)utils_my_realloc_zero (uv1s_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (joints == NULL)
            {
                joints = (unsigned char**)utils_my_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                joints = (unsigned char**)utils_my_realloc_zero (joints, sizeof (unsigned char*) * num_primitives_data, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (joints_sizes == NULL)
            {
                joints_sizes = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                joints_sizes = (size_t*)utils_my_realloc_zero (joints_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (joints_offsets == NULL)
            {
                joints_offsets = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                joints_offsets = (size_t*)utils_my_realloc_zero (joints_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (weights == NULL)
            {
                weights = (unsigned char**)utils_my_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                weights = (unsigned char**)utils_my_realloc_zero (weights,  sizeof (unsigned char*) * num_primitives_data, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (weights_sizes == NULL)
            {
                weights_sizes = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                weights_sizes = (size_t*)utils_my_realloc_zero (weights_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (weights_offsets == NULL)
            {
                weights_offsets = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                weights_offsets = (size_t*)utils_my_realloc_zero (weights_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices == NULL)
            {
                indices = (unsigned char**)utils_my_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                indices = (unsigned char**)utils_my_realloc_zero (indices, sizeof (unsigned char*) * num_primitives_data, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices_counts == NULL)
            {
                indices_counts = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                indices_counts = (size_t*)utils_my_realloc_zero (indices_counts, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices_sizes == NULL)
            {
                indices_sizes = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                indices_sizes = (size_t*)utils_my_realloc_zero (indices_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices_offsets == NULL)
            {
                indices_offsets = (size_t*)utils_my_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                indices_offsets = (size_t*)utils_my_realloc_zero (indices_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices_types == NULL)
            {
                indices_types = (VkIndexType*)utils_my_calloc (current_mesh->primitives_count, sizeof (VkIndexType));
            }
            else
            {
                indices_types = (VkIndexType*)utils_my_realloc_zero (indices_types, sizeof (VkIndexType) * num_primitives_data, sizeof (VkIndexType) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (ref_cgltf_graphics_primitives_for_linking_materials == NULL)
            {
                ref_cgltf_graphics_primitives_for_linking_materials = (cgltf_primitive**)utils_my_calloc (current_mesh->primitives_count, sizeof (cgltf_primitive*));
            }
            else
            {
                ref_cgltf_graphics_primitives_for_linking_materials = (cgltf_primitive**)utils_my_realloc_zero (ref_cgltf_graphics_primitives_for_linking_materials, sizeof (cgltf_primitive*) * num_primitives_data, sizeof (cgltf_primitive*) * (num_primitives_data + current_mesh->primitives_count));
            }

            for (size_t p = 0; p < current_mesh->primitives_count; ++p)
            {
                cgltf_primitive* current_primitive = current_mesh->primitives + p;

                for (size_t a = 0; a < current_primitive->attributes_count; ++a)
                {
                    cgltf_attribute* current_attribute = current_primitive->attributes + a;
                    cgltf_accessor* accessor = current_attribute->data;
                    cgltf_buffer_view* buffer_view = accessor->buffer_view;

                    if (current_attribute->type == cgltf_attribute_type_position)
                    {
                        positions[current_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        positions_sizes[current_primitive_index] = buffer_view->size;
                        positions_offsets[current_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                    else if (current_attribute->type == cgltf_attribute_type_normal)
                    {
                        normals[current_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        normals_sizes[current_primitive_index] = buffer_view->size;
                        normals_offsets[current_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                    else if (current_attribute->type == cgltf_attribute_type_texcoord)
                    {
                        if (strcmp (current_attribute->name, "TEXCOORD_0") == 0)
                        {
                            uv0s[current_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                            uv0s_sizes[current_primitive_index] = buffer_view->size;
                            uv0s_offsets[current_primitive_index] = current_primitive_data_offset;
                            
                            current_primitive_data_offset += buffer_view->size;
                        }
                        else if (strcmp (current_attribute->name, "TEXCOORD_1") == 0)
                        {
                            uv1s[current_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                            uv1s_sizes[current_primitive_index] = buffer_view->size;
                            uv1s_offsets[current_primitive_index] = current_primitive_data_offset;

                            current_primitive_data_offset += buffer_view->size;
                        }
                    }
                    else if (current_attribute->type == cgltf_attribute_type_joints)
                    {
                        joints[current_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        joints_sizes[current_primitive_index] = buffer_view->size;
                        joints_offsets[current_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                    else if (current_attribute->type == cgltf_attribute_type_weights)
                    {
                        weights[current_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        weights_sizes[current_primitive_index] = buffer_view->size;
                        weights_offsets[current_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                }

                cgltf_accessor* accessor = current_primitive->indices;
                cgltf_buffer_view* buffer_view = accessor->buffer_view;
                indices[current_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                indices_counts[current_primitive_index] = accessor->count;
                indices_sizes[current_primitive_index] = buffer_view->size;
                indices_offsets[current_primitive_index] = current_primitive_data_offset;

                switch (accessor->component_type)
                {
                case cgltf_component_type_r_8:
                    indices_types[current_primitive_index] = VK_INDEX_TYPE_UINT8_EXT;
                    break;

                case cgltf_component_type_r_16u:
                    indices_types[current_primitive_index] = VK_INDEX_TYPE_UINT16;
                    break;

                case cgltf_component_type_r_32u:
                    indices_types[current_primitive_index] = VK_INDEX_TYPE_UINT32;
                    break;

                default:
                    break;
                }

                current_primitive_data_offset += buffer_view->size;

                ref_cgltf_graphics_primitives_for_linking_materials[current_primitive_index] = current_primitive;
                
                ++current_primitive_index;
            }

            num_primitives_data += current_mesh->primitives_count;
            num_ref_cgltf_graphics_primitives_for_linking_materials += current_mesh->primitives_count;
        }
    }
    
    AGAINSTRESULT result;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
   
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (
        graphics_device, 
        current_primitive_data_offset, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_SHARING_MODE_EXCLUSIVE, 
        graphics_queue_family_index, 
        &staging_buffer), 
        result);

    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (
        graphics_device,
        &staging_buffer,
        1,
        physical_device_memory_properties,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        &staging_buffer_memory),
        result);

    out_data->graphics_primitives = (vk_skeletal_graphics_primitive*)utils_my_calloc (num_primitives_data, sizeof (vk_skeletal_graphics_primitive));
    out_data->graphics_primitives_count = num_primitives_data;

    for (size_t p = 0; p < num_primitives_data; ++p)
    {
        if (positions_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                positions_offsets[p],
                positions_sizes[p],
                positions[p]),
                result);
        }

        if (normals_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                normals_offsets[p],
                normals_sizes[p],
                normals[p]),
                result);
        }

        if (uv0s_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                uv0s_offsets[p],
                uv0s_sizes[p],
                uv0s[p]),
                result);
        }

        if (uv1s_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                uv1s_offsets[p],
                uv1s_sizes[p],
                uv1s[p]),
                result);
        }

        if (joints_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                joints_offsets[p],
                joints_sizes[p],
                joints[p]),
                result);
        }

        if (weights_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                weights_offsets[p],
                weights_sizes[p],
                weights[p]),
                result);
        }

        if (indices_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                indices_offsets[p],
                indices_sizes[p],
                indices[p]),
                result);
        }

        out_data->graphics_primitives[p].positions_offset = positions_offsets[p];
        out_data->graphics_primitives[p].normals_offset = normals_offsets[p];
        out_data->graphics_primitives[p].uv0s_offset = uv0s_offsets[p];
        out_data->graphics_primitives[p].uv1s_offset = uv1s_offsets[p];
        out_data->graphics_primitives[p].joints_offset = joints_offsets[p];
        out_data->graphics_primitives[p].weights_offset = weights_offsets[p];
        out_data->graphics_primitives[p].index_type = indices_types[p];
        out_data->graphics_primitives[p].indices_count = indices_counts[p];
        out_data->graphics_primitives[p].indices_offset = indices_offsets[p];
    }

    CHECK_AGAINST_RESULT (
        vk_utils_create_buffer (
        graphics_device,
        current_primitive_data_offset,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        graphics_queue_family_index,
        &out_data->vb_ib),
    result);

    CHECK_AGAINST_RESULT (
        vk_utils_allocate_bind_buffer_memory (
            graphics_device,
            &out_data->vb_ib,
            1,
            physical_device_memory_properties,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &out_data->vb_ib_memory
        ),
    result);

    CHECK_AGAINST_RESULT (
        vk_utils_copy_buffer_to_buffer (
            graphics_device,
            common_command_pool,
            graphics_queue,
            staging_buffer,
            out_data->vb_ib,
            current_primitive_data_offset
        ),
        result);

    vk_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_buffer_memory);

    utils_my_free (positions);
    utils_my_free (positions_sizes);
    utils_my_free (positions_offsets);
    utils_my_free (normals);
    utils_my_free (normals_sizes);
    utils_my_free (normals_offsets);
    utils_my_free (uv0s);
    utils_my_free (uv0s_sizes);
    utils_my_free (uv0s_offsets);
    utils_my_free (uv1s);
    utils_my_free (uv1s_sizes);
    utils_my_free (uv1s_offsets);
    utils_my_free (joints);
    utils_my_free (joints_sizes);
    utils_my_free (joints_offsets);
    utils_my_free (weights);
    utils_my_free (weights_sizes);
    utils_my_free (weights_offsets);
    utils_my_free (indices);
    utils_my_free (indices_counts);
    utils_my_free (indices_sizes);
    utils_my_free (indices_offsets);
    utils_my_free (indices_types);

    return 0;
}

int link_graphics_primitives_to_materials (scene_asset_data* out_data)
{
    OutputDebugString (L"link_graphics_primitives_to_materials\n");

    for (size_t m = 0; m < num_ref_cgltf_materials_for_linking_graphics_primitives; ++m)
    {
        cgltf_material* ref_material = ref_cgltf_materials_for_linking_graphics_primitives[m];

        for (size_t gp = 0; gp < num_ref_cgltf_graphics_primitives_for_linking_materials; ++gp) 
        {
            cgltf_material* primitive_material = ref_cgltf_graphics_primitives_for_linking_materials[gp]->material;
            
            if (primitive_material == ref_material)
            {
                vk_skeletal_material* material = out_data->materials + m;

                if (material->graphics_primitives == NULL)
                {
                    material->graphics_primitives = (vk_skeletal_graphics_primitive**)utils_my_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                }
                else
                {
                    material->graphics_primitives = (vk_skeletal_graphics_primitive**)utils_my_realloc_zero (material->graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * material->graphics_primitives_count , sizeof (vk_skeletal_graphics_primitive*) * (material->graphics_primitives_count + 1));
                }

                material->graphics_primitives[material->graphics_primitives_count] = out_data->graphics_primitives + gp;

                ++material->graphics_primitives_count;
            }
        }
    }

    return 0;
}

int import_skins (cgltf_data** datas, size_t num_datas, scene_asset_data* out_data)
{
    OutputDebugString (L"import_skins\n");

    size_t current_skin_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        if (out_data->skins == NULL)
        {
            out_data->skins = (vk_skin*)utils_my_calloc (current_data->skins_count, sizeof (vk_skin));
        }
        else
        {
            out_data->skins = (vk_skin*)utils_my_realloc_zero (out_data->skins, sizeof (vk_skin) * out_data->skins_count , sizeof (vk_skin) * (current_data->skins_count + out_data->skins_count));
        }

        for (size_t s = 0; s < current_data->skins_count; ++s)
        {
            cgltf_skin* current_skin = current_data->skins + s;
            strcpy (out_data->skins[current_skin_index].name, current_skin->name);

            cgltf_buffer_view* inverse_bind_matrices_buffer_view = current_skin->inverse_bind_matrices->buffer_view;

            out_data->skins[current_skin_index].joints = (vk_joint*)utils_my_calloc (current_skin->joints_count, sizeof (vk_joint));
            out_data->skins[current_skin_index].num_joints = current_skin->joints_count;

            for (size_t j = 0; j < current_skin->joints_count; ++j)
            {
                cgltf_node* current_joint = current_skin->joints[j];
                strcpy (out_data->skins[current_skin_index].joints[j].name, current_joint->name);
                cgltf_node_transform_local (current_joint, out_data->skins[current_skin_index].joints[j].local_matrix);
                cgltf_node_transform_world (current_joint, out_data->skins[current_skin_index].joints[j].world_matrix);
                memcpy (out_data->skins[current_skin_index].joints[j].inverse_bind_matrix, (unsigned char*)inverse_bind_matrices_buffer_view->buffer->data + current_skin->inverse_bind_matrices->offset + inverse_bind_matrices_buffer_view->offset + j * sizeof (float) * 16, sizeof (float) * 16);
            }

            ++current_skin_index;
            out_data->skins_count += current_data->skins_count;
        }
    }

    return 0;
}

int import_animations (cgltf_data** datas, size_t num_datas, scene_asset_data* out_data)
{
    OutputDebugString (L"import_animations\n");

    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        for (size_t a = 0; a < current_data->animations_count; ++a)
        {
            cgltf_animation* current_animation = current_data->animations + a;

            for (size_t c = 0; c < current_animation->channels_count; ++c)
            {
                char buff[2048];
                cgltf_animation_channel* current_channel = current_animation->channels + c;
                strcpy (buff, "Name: ");
                strcat (buff, current_animation->name);
                strcat (buff, ", Channel Target Node: ");
                strcat (buff, current_channel->target_node->name);
                strcat (buff, ", ");

                switch (current_channel->target_path)
                {
                case cgltf_animation_path_type_translation:
                    strcat (buff, "Channel target path: Translation");
                    break;

                case cgltf_animation_path_type_rotation:
                    strcat (buff, "Channel target path: Rotation");
                    break;

                case cgltf_animation_path_type_scale:
                    strcat (buff, "Channel target path: Scale");
                    break;

                case cgltf_animation_path_type_weights:
                    strcat (buff, "Channel target path: Weights");
                    break;

                default:
                    break;
                }

                strcat (buff, "\n");
                wchar_t w_buff[2048];
                mbstowcs (w_buff, buff, 2048);
//                OutputDebugString (w_buff);
            }
        }
    }

    return 0;
}

int import_gltf_datas (const char* full_folder_path, cgltf_data** datas, size_t num_datas, scene_asset_data* out_data)
{
    OutputDebugString (L"import_gltf_datas\n");
    AGAINSTRESULT result;
    
    CHECK_AGAINST_RESULT (import_images (full_folder_path, datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_materials (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_graphics_primitives (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (link_graphics_primitives_to_materials (out_data), result);
    CHECK_AGAINST_RESULT (import_skins (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_animations (datas, num_datas, out_data), result);

    return 0;
}

int gather_gltf_datas (const char* full_file_path, cgltf_data** datas, size_t current_data_index)
{
    OutputDebugString (L"gather_gltf_datas\n");
    cgltf_options options = { 0 };
    cgltf_data* data = NULL;

    if (cgltf_parse_file (&options, full_file_path, &data) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    if (cgltf_load_buffers (&options, data, full_file_path) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    if (cgltf_validate (data) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    gltf_datas[current_data_index] = data;
    return 0;
}

int import_gltf_files_from_folder (const char* partial_folder_path, scene_asset_data** out_gltf_data)
{
    OutputDebugString (L"import_gltf_files_from_folder\n");
    
    file_path* file_paths = NULL;
    size_t num_files = 0;
    utils_get_files_in_folder (partial_folder_path, &file_paths, &num_files);

    num_gltf_datas = num_files;
    gltf_datas = (cgltf_data**)utils_my_calloc (num_files, sizeof (cgltf_data*));

    char full_folder_path[MAX_PATH];
    utils_get_full_file_path (partial_folder_path, full_folder_path);

    AGAINSTRESULT result;

    for (size_t f = 0 ; f < num_files; ++f)
    {
        char current_full_file_path[MAX_PATH];
        strcpy (current_full_file_path, full_folder_path);
        strcat (current_full_file_path, file_paths[f].path);
        CHECK_AGAINST_RESULT (gather_gltf_datas (current_full_file_path, gltf_datas, f), result);
    }

    utils_my_free (file_paths);
    file_paths = NULL;

    *out_gltf_data = (scene_asset_data*)utils_my_calloc (1, sizeof (scene_asset_data));

    CHECK_AGAINST_RESULT (import_gltf_datas (full_folder_path, gltf_datas, num_gltf_datas, *out_gltf_data), result);

    for (size_t d = 0; d < num_gltf_datas; ++d)
    {
        cgltf_free (gltf_datas[d]);
    }

    utils_my_free (gltf_datas);
    gltf_datas = NULL;
    num_gltf_datas = 0;

    utils_my_free (ref_cgltf_images_for_materials);
    ref_cgltf_images_for_materials = NULL;
    num_ref_cgltf_images_for_materials = 0;

    utils_my_free (ref_cgltf_materials_for_linking_graphics_primitives);
    ref_cgltf_materials_for_linking_graphics_primitives = NULL;
    num_ref_cgltf_materials_for_linking_graphics_primitives = 0;

    utils_my_free (ref_cgltf_anims_for_meshes);
    ref_cgltf_anims_for_meshes = NULL;
    num_ref_cgltf_anims_for_meshes = 0;

    utils_my_free (ref_cgltf_graphics_primitives_for_linking_materials);
    ref_cgltf_graphics_primitives_for_linking_materials = NULL;
    num_ref_cgltf_graphics_primitives_for_linking_materials = 0;

    return 0;
}

void cleanup_gltf_data (scene_asset_data* gltf_data)
{
    OutputDebugString (L"cleanup_gltf_data\n");

    if (gltf_data)
    {
        if (gltf_data->images)
        {
            for (size_t i = 0; i < gltf_data->images_count; ++i)
            {
                if (gltf_data->images[i] != VK_NULL_HANDLE)
                {
                    vkDestroyImage (graphics_device, gltf_data->images[i], NULL);
                }
                if (gltf_data->image_views[i] != VK_NULL_HANDLE)
                {
                    vkDestroyImageView (graphics_device, gltf_data->image_views[i], NULL);
                }
            }
            utils_my_free (gltf_data->images);
            utils_my_free (gltf_data->image_views);
        }

        vk_utils_destroy_buffer_and_buffer_memory (graphics_device, gltf_data->vb_ib, gltf_data->vb_ib_memory);
        vkFreeMemory (graphics_device, gltf_data->images_memory, NULL);

        for (size_t m = 0; m < gltf_data->materials_count; ++m)
        {
            utils_my_free (gltf_data->materials[m].graphics_primitives);
        }

        vkDestroyDescriptorPool (graphics_device, gltf_data->descriptor_pool, NULL);

        utils_my_free (gltf_data->graphics_primitives);
        utils_my_free (gltf_data->materials);
        
        for (size_t s = 0; s < gltf_data->skins_count; ++s)
        {
            utils_my_free (gltf_data->skins[s].joints);
        }

        utils_my_free (gltf_data->skins);
        utils_my_free (gltf_data->animations);
        utils_my_free (gltf_data->skeletal_meshes);
        utils_my_free (gltf_data);
    }
}

int import_gltf_file (const char* file_path)
{
    cgltf_options options = { 0 };
    cgltf_data* data = NULL;

    if (cgltf_parse_file (&options, file_path, &data) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    if (cgltf_load_buffers (&options, data, file_path) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    if (cgltf_validate (data) != cgltf_result_success)
    {
        return AGAINST_ERROR_GLTF_IMPORT;
    }

    cgltf_free (data);

    return 0;
}