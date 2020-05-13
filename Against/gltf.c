#include "gltf.h"
#include "utils.h"
#include "error.h"
#include "common_graphics.h"
#include "vk_utils.h"
#include "math.hpp"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <stb_image.h>

cgltf_data** gltf_datas = NULL;
size_t gltf_datas_count = 0;

cgltf_image** ref_cgltf_images = NULL;
size_t ref_cgltf_images_count = 0;

cgltf_material** ref_cgltf_materials = NULL;
size_t ref_cgltf_materials_count = 0;

cgltf_animation** ref_cgltf_anims = NULL;
size_t ref_cgltf_anims_count = 0;

cgltf_primitive** ref_cgltf_graphics_primitives = NULL;
size_t ref_cgltf_graphics_primitives_count = 0;

cgltf_node** ref_cgltf_joints = NULL;
size_t ref_cgltf_joints_count = 0;

cgltf_skin** ref_cgltf_skins = NULL;
size_t ref_cgltf_skins_count = 0;

cgltf_node** ref_cgltf_mesh_nodes = NULL;
size_t ref_cgltf_mesh_nodes_count = 0;

typedef struct joint_data
{
    float* translations;
    size_t translations_count;

    float* rotations;
    size_t rotations_count;

    float* matrices;
    size_t matrices_count;
    
    char joint_name[2048];
    char anim_name[2048];
    char skin_name[2048];
} joint_data;

typedef struct anim_joint_data
{
    joint_data* joint_anims;
    size_t joint_anims_count;
    
    size_t frames_count;

    char name[2048];
} anim_joint_data;

AGAINST_RESULT import_images (const char* full_folder_path, cgltf_data** datas, size_t datas_count, scene_asset_data* out_data)
{
    OutputDebugString (L"import_images\n");

    size_t total_image_size = 0;

    size_t* img_offsets = NULL;
    size_t* img_sizes = NULL;
    size_t* img_widths = NULL;
    size_t* img_heights = NULL;
    uint8_t** img_pixels = NULL;

    AGAINST_RESULT result;

    size_t current_index = 0;

    for (size_t d = 0; d < datas_count; ++d)
    {
        cgltf_data* current_data = datas[d];

        if (out_data->images == NULL)
        {
            out_data->images = (VkImage*)utils_calloc (current_data->images_count, sizeof (VkImage));
        }
        else
        {
            out_data->images = (VkImage*)utils_realloc_zero (out_data->images, sizeof (VkImage) * out_data->images_count, sizeof (VkImage) * (out_data->images_count + current_data->images_count));
        }

        if (out_data->image_views == NULL)
        {
            out_data->image_views = (VkImageView*)utils_calloc (current_data->images_count, sizeof (VkImageView));
        }
        else
        {
            out_data->image_views = (VkImageView*)utils_realloc_zero (out_data->image_views, sizeof (VkImageView) * out_data->images_count, sizeof (VkImageView) * (out_data->images_count + current_data->images_count));
        }

        if (img_offsets == NULL)
        {
            img_offsets = (size_t*)utils_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_offsets = (size_t*)utils_realloc_zero (img_offsets, sizeof (size_t) * out_data->images_count, sizeof (size_t) * (out_data->images_count + current_data->images_count));
        }

        if (img_sizes == NULL)
        {
            img_sizes = (size_t*)utils_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_sizes = (size_t*)utils_realloc_zero (img_sizes, sizeof (size_t) * out_data->images_count, sizeof (size_t) * (out_data->images_count + current_data->images_count));
        }

        if (img_widths == NULL)
        {
            img_widths = (size_t*)utils_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_widths = (size_t*)utils_realloc_zero (img_widths, sizeof (size_t) * out_data->images_count, sizeof (size_t) * (out_data->images_count + current_data->images_count));
        }

        if (img_heights == NULL)
        {
            img_heights = (size_t*)utils_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_heights = (size_t*)utils_realloc_zero (img_heights, sizeof (size_t) * out_data->images_count, sizeof (size_t) * (out_data->images_count + current_data->images_count));
        }

        if (img_pixels == NULL)
        {
            img_pixels = (uint8_t**)utils_calloc (current_data->images_count, sizeof (uint8_t*));
        }
        else
        {
            img_pixels = (uint8_t**)utils_realloc_zero (img_pixels, sizeof (uint8_t*) * out_data->images_count, sizeof (uint8_t*) * (out_data->images_count + current_data->images_count));
        }

        if (ref_cgltf_images == NULL)
        {
            ref_cgltf_images = (cgltf_image**)utils_calloc (current_data->images_count, sizeof (cgltf_image*));
        }
        else
        {
            ref_cgltf_images = (cgltf_image**)utils_realloc_zero (ref_cgltf_images, sizeof (cgltf_image*) * out_data->images_count , sizeof (cgltf_image*) * (out_data->images_count + current_data->images_count));
        }

        for (size_t i = 0; i < current_data->images_count; ++i)
        {
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

            ref_cgltf_images[current_index] = image;
            ++current_index;
        }

        ref_cgltf_images_count += current_data->images_count;
        out_data->images_count += current_data->images_count;
    }

    VkBuffer staging_buffer; VkDeviceMemory staging_memory;
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (graphics_device, total_image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, transfer_queue_family_index, &staging_buffer), result);
    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (graphics_device, &staging_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_memory), result);

    for (size_t i = 0; i < ref_cgltf_images_count; ++i)
    {
        CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (graphics_device, staging_memory, img_offsets[i], img_sizes[i], img_pixels[i]), result);
    }

    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_image_memory (graphics_device, out_data->images, ref_cgltf_images_count, physical_device_memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &out_data->images_memory), result);

    for (size_t i = 0; i < ref_cgltf_images_count; ++i)
    {
        VkExtent3D img_extent = { img_widths[i], img_heights[i], 1 };
        CHECK_AGAINST_RESULT (vk_utils_change_image_layout (graphics_device, transfer_queue, transfer_command_pool, transfer_queue_family_index, transfer_queue_family_index, out_data->images[i], 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT), result);
        CHECK_AGAINST_RESULT (vk_utils_copy_buffer_to_image (graphics_device, transfer_command_pool, transfer_queue, img_offsets[i], staging_buffer, &out_data->images[i], img_extent, 1), result);
        CHECK_AGAINST_RESULT (vk_utils_change_image_layout (graphics_device, transfer_queue, transfer_command_pool, transfer_queue_family_index, graphics_queue_family_index, out_data->images[i], 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT), result);
        CHECK_AGAINST_RESULT (vk_utils_create_image_view (graphics_device, out_data->images[i], &out_data->image_views[i]), result);
    }

    utils_free (img_offsets);
    utils_free (img_sizes);
    utils_free (img_widths);
    utils_free (img_heights);
    
    for (size_t i = 0; i < ref_cgltf_images_count; ++i)
    {
        stbi_image_free (img_pixels[i]);
    }
    
    utils_free (img_pixels);

    vk_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_memory);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_materials (cgltf_data** datas, size_t datas_count, scene_asset_data* out_data)
{
    OutputDebugString (L"import_materials\n");

    size_t current_index = 0;

    for (size_t d = 0; d < datas_count; ++d)
    {
        cgltf_data* current_data = datas[d];

        if (out_data->materials == NULL)
        {
            out_data->materials = (vk_skeletal_material*)utils_calloc (current_data->materials_count, sizeof (vk_skeletal_material));
        }
        else
        {
            out_data->materials = (vk_skeletal_material*)utils_realloc_zero (out_data->materials, sizeof (vk_skeletal_material) * out_data->materials_count, sizeof (vk_skeletal_material) * (current_data->materials_count + out_data->materials_count));
        }

        if (ref_cgltf_materials == NULL)
        {
            ref_cgltf_materials = (cgltf_material**)utils_calloc (current_data->materials_count, sizeof (cgltf_material*));
        }
        else
        {
            ref_cgltf_materials = (cgltf_material**)utils_realloc_zero (ref_cgltf_materials, sizeof (cgltf_material*) * out_data->materials_count, sizeof (cgltf_material*) * (out_data->materials_count + current_data->materials_count));
        }

        for (size_t m = 0; m < current_data->materials_count; ++m)
        {
            cgltf_material* material = current_data->materials + m;

            strcpy (out_data->materials[current_index].name, material->name);

            for (size_t i = 0; i < ref_cgltf_images_count; ++i)
            {
                if (material->pbr_metallic_roughness.base_color_texture.texture)
                {
                    if (material->pbr_metallic_roughness.base_color_texture.texture->image == ref_cgltf_images[i])
                    {
                        strcpy (out_data->materials[current_index].base_texture.name, ref_cgltf_images[i]->name);
                        out_data->materials[current_index].base_texture.image = out_data->images + current_index;
                        out_data->materials[current_index].base_texture.image_view = out_data->image_views + current_index;
                    }
                }

                if (material->pbr_metallic_roughness.metallic_roughness_texture.texture)
                {
                    if (material->pbr_metallic_roughness.metallic_roughness_texture.texture->image == ref_cgltf_images[i])
                    {
                        strcpy (out_data->materials[current_index].metalness_roughness_texture.name, ref_cgltf_images[i]->name);
                        out_data->materials[current_index].metalness_roughness_texture.image = out_data->images + current_index;
                        out_data->materials[current_index].metalness_roughness_texture.image_view = out_data->image_views + current_index;
                    }
                }

                if (material->normal_texture.texture)
                {
                    if (material->normal_texture.texture->image == ref_cgltf_images[i])
                    {
                        strcpy (out_data->materials[current_index].normal_texture.name, ref_cgltf_images[i]->name);
                        out_data->materials[current_index].normal_texture.image = out_data->images + current_index;
                        out_data->materials[current_index].normal_texture.image_view = out_data->image_views + current_index;
                    }
                }

                if (material->emissive_texture.texture)
                {
                    if (material->emissive_texture.texture->image == ref_cgltf_images[i])
                    {
                        strcpy (out_data->materials[current_index].emissive_texture.name, ref_cgltf_images[i]->name);
                        out_data->materials[current_index].emissive_texture.image = out_data->images + current_index;
                        out_data->materials[current_index].emissive_texture.image_view = out_data->image_views + current_index;
                    }
                }

                if (material->occlusion_texture.texture)
                {
                    if (material->occlusion_texture.texture->image == ref_cgltf_images[i])
                    {
                        strcpy (out_data->materials[current_index].occlusion_texture.name, ref_cgltf_images[i]->name);
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

            ref_cgltf_materials[current_index] = material;
            ++current_index;
        }

        out_data->materials_count += current_data->materials_count;
        ref_cgltf_materials_count += current_data->materials_count;
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_graphics_primitives (cgltf_data** datas, size_t datas_count, scene_asset_data* out_data)
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

    for (size_t d = 0; d < datas_count; ++d)
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
                positions = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                positions = (unsigned char**)utils_realloc_zero (positions, sizeof (unsigned char*) * num_primitives_data, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (positions_sizes == NULL)
            {
                positions_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                positions_sizes = (size_t*)utils_realloc_zero (positions_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (positions_offsets == NULL)
            {
                positions_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                positions_offsets = (size_t*)utils_realloc_zero (positions_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (normals == NULL)
            {
                normals = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                normals = (unsigned char**)utils_realloc_zero (normals, sizeof (unsigned char*) * num_primitives_data, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (normals_sizes == NULL)
            {
                normals_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                normals_sizes = (size_t*)utils_realloc_zero (normals_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (normals_offsets == NULL)
            {
                normals_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                normals_offsets = (size_t*)utils_realloc_zero (normals_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv0s == NULL)
            {
                uv0s = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                uv0s = (unsigned char**)utils_realloc_zero (uv0s, sizeof (unsigned char*) * num_primitives_data, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv0s_sizes == NULL)
            {
                uv0s_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                uv0s_sizes = (size_t*)utils_realloc_zero (uv0s_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv0s_offsets == NULL)
            {
                uv0s_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                uv0s_offsets = (size_t*)utils_realloc_zero (uv0s_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv1s == NULL)
            {
                uv1s = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                uv1s = (unsigned char**)utils_realloc_zero (uv1s, sizeof (unsigned char*) * num_primitives_data , sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv1s_sizes == NULL)
            {
                uv1s_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                uv1s_sizes = (size_t*)utils_realloc_zero (uv1s_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (uv1s_offsets == NULL)
            {
                uv1s_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                uv1s_offsets = (size_t*)utils_realloc_zero (uv1s_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (joints == NULL)
            {
                joints = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                joints = (unsigned char**)utils_realloc_zero (joints, sizeof (unsigned char*) * num_primitives_data, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (joints_sizes == NULL)
            {
                joints_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                joints_sizes = (size_t*)utils_realloc_zero (joints_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (joints_offsets == NULL)
            {
                joints_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                joints_offsets = (size_t*)utils_realloc_zero (joints_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (weights == NULL)
            {
                weights = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                weights = (unsigned char**)utils_realloc_zero (weights,  sizeof (unsigned char*) * num_primitives_data, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (weights_sizes == NULL)
            {
                weights_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                weights_sizes = (size_t*)utils_realloc_zero (weights_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (weights_offsets == NULL)
            {
                weights_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                weights_offsets = (size_t*)utils_realloc_zero (weights_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices == NULL)
            {
                indices = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                indices = (unsigned char**)utils_realloc_zero (indices, sizeof (unsigned char*) * num_primitives_data, sizeof (unsigned char*) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices_counts == NULL)
            {
                indices_counts = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                indices_counts = (size_t*)utils_realloc_zero (indices_counts, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices_sizes == NULL)
            {
                indices_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                indices_sizes = (size_t*)utils_realloc_zero (indices_sizes, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices_offsets == NULL)
            {
                indices_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                indices_offsets = (size_t*)utils_realloc_zero (indices_offsets, sizeof (size_t) * num_primitives_data, sizeof (size_t) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (indices_types == NULL)
            {
                indices_types = (VkIndexType*)utils_calloc (current_mesh->primitives_count, sizeof (VkIndexType));
            }
            else
            {
                indices_types = (VkIndexType*)utils_realloc_zero (indices_types, sizeof (VkIndexType) * num_primitives_data, sizeof (VkIndexType) * (num_primitives_data + current_mesh->primitives_count));
            }

            if (ref_cgltf_graphics_primitives == NULL)
            {
                ref_cgltf_graphics_primitives = (cgltf_primitive**)utils_calloc (current_mesh->primitives_count, sizeof (cgltf_primitive*));
            }
            else
            {
                ref_cgltf_graphics_primitives = (cgltf_primitive**)utils_realloc_zero (ref_cgltf_graphics_primitives, sizeof (cgltf_primitive*) * num_primitives_data, sizeof (cgltf_primitive*) * (num_primitives_data + current_mesh->primitives_count));
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

                ref_cgltf_graphics_primitives[current_primitive_index] = current_primitive;
                
                ++current_primitive_index;
            }

            num_primitives_data += current_mesh->primitives_count;
            ref_cgltf_graphics_primitives_count += current_mesh->primitives_count;
        }
    }
    
    AGAINST_RESULT result;
    
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

    out_data->graphics_primitives = (vk_skeletal_graphics_primitive*)utils_calloc (num_primitives_data, sizeof (vk_skeletal_graphics_primitive));
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
            transfer_command_pool,
            transfer_queue,
            staging_buffer,
            out_data->vb_ib,
            current_primitive_data_offset
        ),
        result);

    vk_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_buffer_memory);

    utils_free (positions);
    utils_free (positions_sizes);
    utils_free (positions_offsets);
    utils_free (normals);
    utils_free (normals_sizes);
    utils_free (normals_offsets);
    utils_free (uv0s);
    utils_free (uv0s_sizes);
    utils_free (uv0s_offsets);
    utils_free (uv1s);
    utils_free (uv1s_sizes);
    utils_free (uv1s_offsets);
    utils_free (joints);
    utils_free (joints_sizes);
    utils_free (joints_offsets);
    utils_free (weights);
    utils_free (weights_sizes);
    utils_free (weights_offsets);
    utils_free (indices);
    utils_free (indices_counts);
    utils_free (indices_sizes);
    utils_free (indices_offsets);
    utils_free (indices_types);

    return AGAINST_SUCCESS;
}

/*AGAINST_RESULT link_graphics_primitives_to_materials (scene_asset_data* out_data)
{
    OutputDebugString (L"link_graphics_primitives_to_materials\n");

    for (size_t m = 0; m < ref_cgltf_materials_count; ++m)
    {
        cgltf_material* ref_material = ref_cgltf_materials[m];

        for (size_t gp = 0; gp < ref_cgltf_graphics_primitives_count; ++gp) 
        {
            cgltf_material* primitive_material = ref_cgltf_graphics_primitives[gp]->material;
            
            if (primitive_material == ref_material)
            {
                vk_skeletal_material* material = out_data->materials + m;

                if (material->graphics_primitives == NULL)
                {
                    material->graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                }
                else
                {
                    material->graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (material->graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * material->graphics_primitives_count , sizeof (vk_skeletal_graphics_primitive*) * (material->graphics_primitives_count + 1));
                }

                material->graphics_primitives[material->graphics_primitives_count] = out_data->graphics_primitives + gp;

                ++material->graphics_primitives_count;
            }
        }
    }

    return AGAINST_SUCCESS;
}*/

AGAINST_RESULT link_materials_to_graphics_primitives (scene_asset_data* out_data)
{
    OutputDebugString (L"link_materials_to_graphics_primitives\n");

    for (size_t gp = 0; gp < ref_cgltf_graphics_primitives_count; ++gp)
    {
        cgltf_primitive* current_primitive = ref_cgltf_graphics_primitives[gp];

        for (size_t m = 0; m < ref_cgltf_materials_count; ++m)
        {
            cgltf_material* current_material = ref_cgltf_materials[m];

            if (current_primitive->material == current_material)
            {
                out_data->graphics_primitives[gp].material = out_data->materials + m;
            }
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_skins (cgltf_data** datas, size_t datas_count, scene_asset_data* out_data)
{
    OutputDebugString (L"import_skins\n");

    size_t skins_count = 0;
    size_t current_skin_index = 0;

    for (size_t d = 0; d < datas_count; ++d)
    {
        cgltf_data* current_data = datas[d];

        if (ref_cgltf_skins == NULL)
        {
            ref_cgltf_skins = (cgltf_skin**)utils_calloc (current_data->skins_count, sizeof (cgltf_skin*));
        }
        else
        {
            ref_cgltf_skins = (cgltf_skin**)utils_realloc_zero (ref_cgltf_skins, sizeof (cgltf_skin*) * out_data->skins_count, sizeof (cgltf_skin*) * (out_data->skins_count + current_data->skins_count));
        }

        if (out_data->skins == NULL)
        {
            out_data->skins = (vk_skin*)utils_calloc (current_data->skins_count, sizeof (vk_skin));
        }
        else
        {
            out_data->skins = (vk_skin*)utils_realloc_zero (out_data->skins, sizeof (vk_skin) * out_data->skins_count, sizeof (vk_skin) * (out_data->skins_count + current_data->skins_count));
        }

        for (size_t s = 0; s < current_data->skins_count; ++s)
        {
            ref_cgltf_skins[current_skin_index] = current_data->skins + s;
            ++current_skin_index;
        }

        ref_cgltf_skins_count += current_data->skins_count;
        out_data->skins_count += current_data->skins_count;
        skins_count += current_data->skins_count;
    }

    size_t skin_joints_size = MAX_JOINTS * sizeof (float) * 16;
    size_t min_ubo_alignment = (size_t)physical_device_limits.minUniformBufferOffsetAlignment;
    size_t aligned_skin_joints_size = 0;
    vk_utils_get_aligned_size (skin_joints_size, min_ubo_alignment, &aligned_skin_joints_size);
    
    size_t total_data_size = aligned_skin_joints_size * skins_count;

    float* skin_joints_matrices = (float*)utils_aligned_malloc_zero (total_data_size, min_ubo_alignment);

    for (size_t s = 0; s < ref_cgltf_skins_count; ++s)
    {
        cgltf_skin* current_skin = ref_cgltf_skins[s];

        for (size_t j = 0; j < current_skin->joints_count; ++j)
        {
            cgltf_node* current_joint = current_skin->joints[j];

            wchar_t buff[256];
            swprintf (buff, 256, L"%hs\n", current_joint->name);
            //OutputDebugString (buff);

            float matrix[16];
            cgltf_node_transform_world (current_joint, matrix);
            memcpy ((unsigned char*)skin_joints_matrices + (aligned_skin_joints_size * s) + (sizeof (float) * 16 * j), matrix, sizeof (float) * 16);
        }

        strcpy (out_data->skins[s].name, current_skin->name);
        out_data->skins[s].bind_pose_offset = (VkDeviceSize)aligned_skin_joints_size * (VkDeviceSize)s;
    }

    AGAINST_RESULT result;

    VkBuffer staging_buffer; VkDeviceMemory staging_buffer_memory;
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (graphics_device, total_data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &staging_buffer), result);
    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (graphics_device, &staging_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_buffer_memory), result);

    CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (graphics_device, staging_buffer_memory, 0, total_data_size, skin_joints_matrices), result);

    CHECK_AGAINST_RESULT (vk_utils_create_buffer (graphics_device, total_data_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &out_data->bone_buffer), result);
    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (graphics_device, &out_data->bone_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &out_data->bone_buffer_memory), result);
    CHECK_AGAINST_RESULT (vk_utils_copy_buffer_to_buffer (graphics_device, transfer_command_pool, transfer_queue, staging_buffer, out_data->bone_buffer, total_data_size), result);

    vk_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_buffer_memory);

    utils_aligned_free (skin_joints_matrices);

    return AGAINST_SUCCESS;
}

/*AGAINST_RESULT import_skin_animations (cgltf_data** datas, size_t datas_count, scene_asset_data* out_data)
{
    OutputDebugString (L"import_skin_animations\n");

    size_t total_data_size = 0;
    size_t current_animation_index = 0;

    for (size_t s = 0; s < ref_cgltf_skins_count; ++s)
    {
        cgltf_skin* current_skin = ref_cgltf_skins[s];

        for (size_t d = 0; d < datas_count; ++d)
        {
            cgltf_data* current_data = datas[d];

            for (size_t a = 0; a < current_data->animations_count; ++a)
            {
                cgltf_animation* current_anim = current_data->animations + a;

                if (strstr (current_anim->name, current_skin->name) != NULL)
                {
                    size_t frames_count = 0;
                    unsigned char* all_frames_data = NULL;
                    size_t per_frame_data_size = 0;

                    for (size_t j = 0; j < current_skin->joints_count; ++j)
                    {
                        cgltf_node* current_joint = current_skin->joints[j];

                        float* joint_translations = NULL;
                        size_t num_frames_translation = 0;
                        float* joint_rotations = NULL;
                        size_t num_frames_rotation = 0;

                        for (size_t c = 0; c < current_anim->channels_count; ++c)
                        {
                            cgltf_animation_channel* current_channel = current_anim->channels + c;

                            frames_count = current_channel->sampler->input->count;
                            per_frame_data_size = MAX_JOINTS * 16 * sizeof (float);

                            if (strcmp (current_channel->target_node->name, current_joint->name) == 0)
                            {
                                if (current_channel->target_path == cgltf_animation_path_type_translation)
                                {
                                    unsigned char* data_ptr = (unsigned char*) current_channel->sampler->output->buffer_view->buffer->data + current_channel->sampler->output->offset + current_channel->sampler->output->buffer_view->offset; 
                                    joint_translations = (float*) utils_malloc_zero (current_channel->sampler->output->buffer_view->size);
                                    memcpy (joint_translations, data_ptr, current_channel->sampler->output->buffer_view->size);
                                    num_frames_translation = current_channel->sampler->output->count;
                                }
                                else if (current_channel->target_path == cgltf_animation_path_type_rotation)
                                {
                                    unsigned char* data_ptr = (unsigned char*) current_channel->sampler->output->buffer_view->buffer->data + current_channel->sampler->output->offset + current_channel->sampler->output->buffer_view->offset; 
                                    joint_rotations = (float*) utils_malloc_zero (current_channel->sampler->output->buffer_view->size);
                                    memcpy (joint_rotations, data_ptr, current_channel->sampler->output->buffer_view->size);
                                    num_frames_rotation = current_channel->sampler->output->count;
                                }
                            }
                        }

                        size_t max_frames = num_frames_translation > num_frames_rotation ? num_frames_translation : num_frames_rotation;
                        
                        float* joint_matrices = (float*) utils_calloc (max_frames * 16, sizeof(float));
                        float joint_matrix[16];
                        
                        for (size_t f = 0; f < max_frames; ++f)
                        {
                            math_create_identity_matrix(joint_matrix);
                            if (f < num_frames_translation)
                            {
                                math_translate_matrix (joint_matrix, joint_translations + (f * 3), joint_matrix);
                            }
                            if (f < num_frames_rotation)
                            {
                                math_rotate_matrix (joint_matrix, joint_rotations + (f * 4), joint_matrix);
                            }
                            memcpy (joint_matrices + (f * 16), joint_matrix, sizeof (float) * 16);
                        }

                        utils_free (joint_matrices);
                        utils_free (joint_translations);
                        utils_free (joint_rotations);
                    }

                    ++out_data->skins[s].animations_count;
                }
            }
        }
    }

    return AGAINST_SUCCESS;
}

/*AGAINST_RESULT gather_animations (cgltf_data** datas, size_t datas_count)
{
    OutputDebugString (L"gather_animations\n");

    size_t current_anim_index = 0;

    for (size_t d = 0; d < datas_count; ++d)
    {
        cgltf_data* current_data = datas[d];
        if (ref_cgltf_anims == NULL)
        {
            ref_cgltf_anims = (cgltf_animation**)utils_calloc (current_data->animations_count, sizeof (cgltf_animation*));
        }
        else
        {
            ref_cgltf_anims = (cgltf_animation**)utils_realloc_zero (ref_cgltf_anims, sizeof (cgltf_animation*) * ref_cgltf_anims_count, sizeof (cgltf_animation*) * (ref_cgltf_anims_count + current_data->animations_count));
        }

        for (size_t a = 0; a < current_data->animations_count; ++a)
        {
            cgltf_animation* current_animation = current_data->animations + a;
            ref_cgltf_anims[current_anim_index] = current_animation;
            ++current_anim_index;
        }

        ref_cgltf_anims_count += current_data->animations_count;
    }

    return AGAINST_SUCCESS;
}*/

/*AGAINST_RESULT import_animations (cgltf_data** datas, size_t datas_count, scene_asset_data* out_data)
{
    OutputDebugString (L"import_animations\n");

    skin_data* skin_anims = (skin_data*)utils_calloc (ref_cgltf_skins_count, sizeof (skin_data));

    for (size_t s = 0; s < ref_cgltf_skins_count; ++s)
    {
        cgltf_skin* current_skin = ref_cgltf_skins[s];

        for (size_t a = 0; a < ref_cgltf_anims_count; ++a)
        {
            cgltf_animation* current_animation = ref_cgltf_anims[a];
            if (strstr (current_animation->name, current_skin->name) != NULL)
            {
                if (skin_anims[s].anim_anims == NULL)
                {
                    skin_anims[s].anim_anims = (anim_joint_data*)utils_calloc (1, sizeof (anim_joint_data));
                }
                else
                {
                    skin_anims[s].anim_anims = (anim_joint_data*)utils_realloc_zero (skin_anims[s].anim_anims, sizeof (anim_joint_data) * skin_anims[s].anim_anims_count, sizeof (anim_joint_data) * (skin_anims[s].anim_anims_count + 1));
                }
                
                for (size_t j = 0; j < current_skin->joints_count; ++j)
                {
                    cgltf_node* current_joint = current_skin->joints[j];

                    if (skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims == NULL)
                    {
                        skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims = (joint_data*)utils_calloc (1, sizeof (joint_data));
                    }
                    else
                    {
                        skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims = (joint_data*)utils_realloc_zero (skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims, sizeof (joint_data) * skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims_count, sizeof (joint_data) * (skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims_count + 1));
                    }
                    
                    for (size_t c = 0; c < current_animation->channels_count; ++c)
                    {
                        cgltf_animation_channel* current_channel = current_animation->channels + c;

                        if (strcmp (current_channel->target_node->name, current_joint->name) == 0)
                        {
                            if (current_channel->target_path == cgltf_animation_path_type_translation)
                            {
                                skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims[skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims_count].translations = (float*)((unsigned char*)current_channel->sampler->output->buffer_view->buffer->data + current_channel->sampler->output->offset + current_channel->sampler->output->buffer_view->offset);
                                skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims[skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims_count].translations_count = current_channel->sampler->output->count;
                            }
                            
                            if (current_channel->target_path == cgltf_animation_path_type_rotation)
                            {
                                skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims[skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims_count].rotations = (float*)((unsigned char*)current_channel->sampler->output->buffer_view->buffer->data + current_channel->sampler->output->offset + current_channel->sampler->output->buffer_view->offset);
                                skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims[skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims_count].rotations_count = current_channel->sampler->output->count;
                            }

                        }
                    }
                    ++skin_anims[s].anim_anims[skin_anims[s].anim_anims_count].joint_anims_count;
                }
                ++skin_anims[s].anim_anims_count;
            }
        }
    }

    for (size_t sa = 0; sa < ref_cgltf_skins_count; ++sa)
    {
        skin_data* current_skin_anim = skin_anims + sa;
        for (size_t aa = 0; aa < skin_anims[sa].anim_anims_count; ++aa)
        {
            anim_joint_data* current_anim_anim = current_skin_anim->anim_anims + aa;
            for (size_t ja = 0; ja < current_anim_anim->joint_anims_count; ++ja)
            {
                joint_data* current_joint_anim = current_anim_anim->joint_anims + ja;

                utils_free (current_joint_anim->translations);
                utils_free (current_joint_anim->rotations);
            }
            
            utils_free (current_anim_anim->joint_anims);
        }
        utils_free (current_skin_anim->anim_anims);
    }
    utils_free (skin_anims);

    return AGAINST_SUCCESS;
}*/

AGAINST_RESULT import_animations (cgltf_data** datas, size_t datas_count, scene_asset_data* out_data)
{
    OutputDebugString (L"import_animations\n");

    anim_joint_data* anim_joint_datas = NULL;
    size_t anim_joint_datas_count = 0;

    for (size_t d = 0; d < datas_count; ++d)
    {
        cgltf_data* current_data = datas[d];

        for (size_t a = 0; a < current_data->animations_count; ++a)
        {
            if (anim_joint_datas == NULL)
            {
                anim_joint_datas = (anim_joint_data*)utils_calloc (current_data->animations_count, sizeof (anim_joint_data));
            }
            else
            {
                anim_joint_datas = (anim_joint_data*)utils_realloc_zero (anim_joint_datas, sizeof (anim_joint_data) * anim_joint_datas_count, sizeof (anim_joint_data) * (anim_joint_datas_count + 1));
            }

            cgltf_animation* current_animation = current_data->animations + a;
            anim_joint_data* current_ajd = anim_joint_datas + anim_joint_datas_count;

            strcpy (current_ajd->name, current_animation->name);

            for (size_t c = 0; c < current_animation->channels_count; ++c)
            {
                cgltf_animation_channel* current_channel = current_animation->channels + c;
                size_t joint_data_index = -1;
                for (size_t ja = 0; ja < current_ajd->joint_anims_count; ++ja)
                {
                    if (strcmp (current_channel->target_node->name, current_ajd->joint_anims[ja].joint_name) == 0)
                    {
                        joint_data_index = ja;
                        break;
                    }
                }

                if (joint_data_index == -1)
                {
                    if (current_ajd->joint_anims == NULL)
                    {
                        current_ajd->joint_anims = (joint_data*)utils_calloc (1, sizeof (joint_data));
                    }
                    else
                    {
                        current_ajd->joint_anims = (joint_data*)utils_realloc_zero (current_ajd->joint_anims, sizeof (joint_data) * current_ajd->joint_anims_count, sizeof (joint_data) * (current_ajd->joint_anims_count + 1));
                    }

                    joint_data_index = current_ajd->joint_anims_count;
                    ++current_ajd->joint_anims_count;
                }

                joint_data* active_jd = current_ajd->joint_anims + joint_data_index;

                strcpy (active_jd->joint_name, current_channel->target_node->name);
                strcpy (active_jd->anim_name, current_animation->name);

                if (current_channel->target_path == cgltf_animation_path_type_translation)
                {
                    active_jd->translations = (float*)((unsigned char*)current_channel->sampler->output->buffer_view->buffer->data + current_channel->sampler->output->offset + current_channel->sampler->output->buffer_view->offset);
                    active_jd->translations_count = current_channel->sampler->output->count;
                }

                if (current_channel->target_path == cgltf_animation_path_type_rotation)
                {
                    active_jd->rotations = (float*)((unsigned char*)current_channel->sampler->output->buffer_view->buffer->data + current_channel->sampler->output->offset + current_channel->sampler->output->buffer_view->offset);
                    active_jd->rotations_count = current_channel->sampler->output->count;
                }
            }

            ++anim_joint_datas_count;
        }
    }

    for (size_t ajd = 0; ajd < anim_joint_datas_count; ++ajd)
    {
        anim_joint_data* current_ajd = anim_joint_datas + ajd;
        for (size_t jd = 0; jd < current_ajd->joint_anims_count; ++jd)
        {
            joint_data* current_jd = current_ajd->joint_anims + jd;

            current_jd->matrices_count = current_jd->translations_count >= current_jd->rotations_count ? current_jd->translations_count : current_jd->rotations_count;
            current_jd->matrices = (float*)utils_malloc_zero (sizeof (float) * 16 * current_jd->matrices_count);

            current_ajd->frames_count = current_jd->matrices_count >= current_ajd->frames_count ? current_jd->matrices_count : current_ajd->frames_count;

            float matrix[16];

            for (size_t f = 0; f < current_jd->matrices_count; ++f)
            {
                math_create_identity_matrix (matrix);
                math_translate_matrix (matrix, current_jd->translations + (f * 3), matrix);
                math_rotate_matrix (matrix, current_jd->rotations + (f * 4), matrix);

                memcpy (current_jd->matrices + (f * 16), matrix, sizeof (float) * 16);
            }
        }
    }

    out_data->animations_count = anim_joint_datas_count;
    out_data->animations = (vk_animation*)utils_calloc (out_data->animations_count, sizeof (vk_animation));

    size_t size_of_single_frame = MAX_JOINTS * 16 * sizeof (float);
    size_t aligned_size_of_single_frame = 0;
    vk_utils_get_aligned_size (size_of_single_frame, (size_t)physical_device_limits.minUniformBufferOffsetAlignment, &aligned_size_of_single_frame);

    unsigned char* all_animations_aligned_memory = NULL;
    size_t total_data_allocated = 0;

    for (size_t ajd = 0; ajd < anim_joint_datas_count; ++ajd)
    {
        anim_joint_data* current_ajd = anim_joint_datas + ajd;
        vk_animation* current_out_anim = out_data->animations + ajd;

        strcpy (current_out_anim->name, current_ajd->name);
        current_out_anim->frames_count = current_ajd->frames_count;
        current_out_anim->frame_data_offsets = (VkDeviceSize*)utils_calloc (current_out_anim->frames_count, sizeof (VkDeviceSize));

        if (all_animations_aligned_memory == NULL)
        {
            all_animations_aligned_memory = (unsigned char*)utils_aligned_malloc_zero (size_of_single_frame * current_out_anim->frames_count, (size_t)physical_device_limits.minUniformBufferOffsetAlignment);
        }
        else
        {
            all_animations_aligned_memory = (unsigned char*)utils_aligned_realloc_zero (all_animations_aligned_memory, (size_t)physical_device_limits.minUniformBufferOffsetAlignment, total_data_allocated, total_data_allocated + (size_of_single_frame * current_out_anim->frames_count));
        }
            
        for (size_t f = 0; f < current_ajd->frames_count; ++f)
        {
            for (size_t jd = 0; jd < current_ajd->joint_anims_count; ++jd)
            {
                joint_data* current_jd = current_ajd->joint_anims + jd;
                float* joint_matrix = current_jd->matrices + (f * 16);

                memcpy (all_animations_aligned_memory + (ajd * size_of_single_frame) + (jd * sizeof (float) * 16), joint_matrix, sizeof (float) * 16);
            }

            current_out_anim->frame_data_offsets[f] = total_data_allocated;
        }

        total_data_allocated += size_of_single_frame * current_out_anim->frames_count;
    }

    AGAINST_RESULT result;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (graphics_device, total_data_allocated, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &staging_buffer), result);
    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (graphics_device, &staging_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_buffer_memory), result);
    CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (graphics_device, staging_buffer_memory, 0, total_data_allocated, all_animations_aligned_memory), result);
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (graphics_device, total_data_allocated, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &out_data->anim_buffer), result);
    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (graphics_device, &out_data->anim_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &out_data->anim_buffer_memory), result);
    CHECK_AGAINST_RESULT (vk_utils_copy_buffer_to_buffer (graphics_device, transfer_command_pool, transfer_queue, staging_buffer, out_data->anim_buffer, total_data_allocated), result);

    vk_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_buffer_memory);

    for (size_t ajd = 0; ajd < anim_joint_datas_count; ++ajd)
    {
        for (size_t jd = 0; jd < anim_joint_datas[ajd].joint_anims_count; ++jd)
        {
            utils_free (anim_joint_datas[ajd].joint_anims[jd].matrices);
        }
        utils_free (anim_joint_datas[ajd].joint_anims);
    }
    utils_free (anim_joint_datas);

    utils_aligned_free (all_animations_aligned_memory);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_skeletal_meshes (cgltf_data** datas, size_t datas_count, scene_asset_data* out_data)
{
    OutputDebugString (L"import_skeletal_meshes\n");

    size_t current_mesh_node_index = 0;

    for (size_t d = 0; d < datas_count; ++d)
    {
        cgltf_data* current_data = datas[d];
        
        for (size_t n = 0; n < current_data->nodes_count; ++n)
        {
            cgltf_node* current_node = current_data->nodes + n;

            if (strstr (current_node->name, "CS_") != NULL)
            {
                continue;
            }

            if (current_node->mesh == NULL)
            {
                continue;
            }

            if (out_data->skeletal_meshes == NULL)
            {
                out_data->skeletal_meshes = (vk_skeletal_mesh*)utils_calloc (1, sizeof (vk_skeletal_mesh));
            }
            else
            {
                out_data->skeletal_meshes = (vk_skeletal_mesh*)utils_realloc_zero (out_data->skeletal_meshes, sizeof (vk_skeletal_mesh) * out_data->skeletal_meshes_count, sizeof (vk_skeletal_mesh) * (out_data->skeletal_meshes_count + 1));
            }

            if (ref_cgltf_mesh_nodes == NULL)
            {
                ref_cgltf_mesh_nodes = (cgltf_node**)utils_calloc (1, sizeof (cgltf_node*));
            }
            else
            {
                ref_cgltf_mesh_nodes = (cgltf_node**)utils_realloc_zero (ref_cgltf_mesh_nodes, sizeof (cgltf_node*) * out_data->skeletal_meshes_count, sizeof (cgltf_node*) * (out_data->skeletal_meshes_count + 1));
            }

            strcpy (out_data->skeletal_meshes[current_mesh_node_index].name, current_node->name);

            ref_cgltf_mesh_nodes[current_mesh_node_index] = current_node;
            ++ref_cgltf_mesh_nodes_count;
            ++out_data->skeletal_meshes_count;
            ++current_mesh_node_index;
        }
    }

    return AGAINST_SUCCESS;
}

/*AGAINST_RESULT link_materials_to_meshes (scene_asset_data* out_data)
{
    OutputDebugString (L"link_materials_to_meshes\n");

    for (size_t m = 0; m < ref_cgltf_materials_count; ++m)
    {
        cgltf_material* current_material = ref_cgltf_materials[m];

        for (size_t n = 0; n < ref_cgltf_mesh_nodes_count; ++n)
        {
            if (strstr (ref_cgltf_mesh_nodes[n]->name, "CS_") != NULL)
            {
                continue;
            }

            if (ref_cgltf_mesh_nodes[n]->mesh == NULL)
            {
                continue;
            }

            cgltf_mesh* current_mesh = ref_cgltf_mesh_nodes[n]->mesh;
            for (size_t gp = 0; gp < current_mesh->primitives_count; ++gp)
            {
                if (current_material == current_mesh->primitives[gp].material)
                {
                    if (strstr (current_material->name, "opaque") != NULL)
                    {
                        if (out_data->skeletal_meshes[n].opaque_graphics_primitives == NULL)
                        {
                            out_data->skeletal_meshes[n].opaque_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                        }
                        else
                        {
                            out_data->skeletal_meshes[n].opaque_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (out_data->skeletal_meshes[n].opaque_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skeletal_meshes[n].opaque_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skeletal_meshes[n].opaque_graphics_primitives_count + 1));
                        }

                        out_data->skeletal_meshes[n].opaque_graphics_primitives[out_data->skeletal_meshes[n].opaque_graphics_primitives_count] = out_data->materials + m;
                        ++out_data->skeletal_meshes[n].opaque_graphics_primitives_count;
                    }

                    if (strstr (current_material->name, "alpha") != NULL)
                    {
                        if (out_data->skeletal_meshes[n].alpha_graphics_primitives == NULL)
                        {
                            out_data->skeletal_meshes[n].alpha_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                        }
                        else
                        {
                            out_data->skeletal_meshes[m].alpha_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (out_data->skeletal_meshes[n].alpha_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skeletal_meshes[n].alpha_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skeletal_meshes[n].alpha_graphics_primitives_count + 1));
                        }
                        
                        out_data->skeletal_meshes[n].alpha_graphics_primitives[out_data->skeletal_meshes[n].alpha_graphics_primitives_count] = out_data->materials + m;
                        ++out_data->skeletal_meshes[n].alpha_graphics_primitives_count;
                    }

                    if (strstr (current_material->name, "blend") != NULL)
                    {
                        if (out_data->skeletal_meshes[n].blend_graphics_primitives == NULL)
                        {
                            out_data->skeletal_meshes[n].blend_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_material*));
                        }
                        else
                        {
                            out_data->skeletal_meshes[n].blend_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (out_data->skeletal_meshes[n].blend_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skeletal_meshes[n].blend_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skeletal_meshes[n].blend_graphics_primitives_count + 1));
                        }

                        out_data->skeletal_meshes[n].blend_graphics_primitives[out_data->skeletal_meshes[n].blend_graphics_primitives_count] = out_data->materials + m;
                        ++out_data->skeletal_meshes[n].blend_graphics_primitives_count;
                    }
                }
            }
        }
    }

    return AGAINST_SUCCESS;
}*/

/*AGAINST_RESULT link_graphics_primitives_to_meshes (scene_asset_data* out_data)
{
    OutputDebugString (L"link_graphics_primitives_to_meshes\n");

    for (size_t m = 0; m < ref_cgltf_mesh_nodes_count; ++m)
    {
        cgltf_node* current_mesh_node = ref_cgltf_mesh_nodes[m];

        for (size_t gp = 0; gp < current_mesh_node->mesh->primitives_count; ++gp)
        {
            cgltf_primitive* current_gp = current_mesh_node->mesh->primitives + gp;
            
            for (size_t rgp = 0; rgp < ref_cgltf_graphics_primitives_count; ++rgp)
            {
                if (current_gp == ref_cgltf_graphics_primitives[rgp])
                {
                    if (strstr (current_gp->material->name, "opaque") != NULL)
                    {
                        if (out_data->skeletal_meshes[m].opaque_graphics_primitives == NULL)
                        {
                            out_data->skeletal_meshes[m].opaque_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                        }
                        else
                        {
                            out_data->skeletal_meshes[m].opaque_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (out_data->skeletal_meshes[m].opaque_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skeletal_meshes[m].opaque_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skeletal_meshes[m].opaque_graphics_primitives_count + 1));
                        }

                        out_data->skeletal_meshes[m].opaque_graphics_primitives[out_data->skeletal_meshes[m].opaque_graphics_primitives_count] = out_data->graphics_primitives + rgp;
                        ++out_data->skeletal_meshes[m].opaque_graphics_primitives_count;
                    }

                    if (strstr (current_gp->material->name, "alpha") != NULL)
                    {
                        if (out_data->skeletal_meshes[m].alpha_graphics_primitives == NULL)
                        {
                            out_data->skeletal_meshes[m].alpha_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                        }
                        else
                        {
                            out_data->skeletal_meshes[m].alpha_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (out_data->skeletal_meshes[m].alpha_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skeletal_meshes[m].alpha_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skeletal_meshes[m].alpha_graphics_primitives_count + 1));
                        }

                        out_data->skeletal_meshes[m].alpha_graphics_primitives[out_data->skeletal_meshes[m].alpha_graphics_primitives_count] = out_data->graphics_primitives + rgp;
                        ++out_data->skeletal_meshes[m].alpha_graphics_primitives_count;
                    }

                    if (strstr (current_gp->material->name, "blend") != NULL)
                    {
                        if (out_data->skeletal_meshes[m].blend_graphics_primitives == NULL)
                        {
                            out_data->skeletal_meshes[m].blend_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                        }
                        else
                        {
                            out_data->skeletal_meshes[m].blend_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (out_data->skeletal_meshes[m].blend_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skeletal_meshes[m].blend_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skeletal_meshes[m].blend_graphics_primitives_count + 1));
                        }

                        out_data->skeletal_meshes[m].blend_graphics_primitives[out_data->skeletal_meshes[m].blend_graphics_primitives_count] = out_data->graphics_primitives + rgp;
                        ++out_data->skeletal_meshes[m].blend_graphics_primitives_count;
                    }
                }
            }
        }
    }

    return AGAINST_SUCCESS;
}*/

/*AGAINST_RESULT link_mesh_node_graphics_primitives_to_skins (scene_asset_data* out_data)
{
    OutputDebugString (L"link_mesh_node_graphics_primitives_to_skins\n");
    
    for (size_t s = 0; s < ref_cgltf_skins_count; ++s)
    {
        cgltf_skin* current_skin = ref_cgltf_skins[s];

        for (size_t n = 0; n < ref_cgltf_mesh_nodes_count; ++n)
        {
            if (ref_cgltf_mesh_nodes[n]->skin == NULL)
            {
                continue;
            }

            cgltf_node* current_node = ref_cgltf_mesh_nodes[n];

            if (current_skin == current_node->skin)
            {
                for (size_t gp = 0; gp < current_node->mesh->primitives_count; ++gp)
                {
                    cgltf_primitive* current_primitive = current_node->mesh->primitives + gp;

                    for (size_t rgp = 0; rgp < ref_cgltf_graphics_primitives_count; ++rgp)
                    {
                        cgltf_primitive* current_rgp = ref_cgltf_graphics_primitives[rgp];
                        
                        if (current_rgp == current_primitive)
                        {
                            if (strstr (current_primitive->material->name, "opaque") != NULL)
                            {
                                if (out_data->skins[s].opaque_graphics_primitives == NULL)
                                {
                                    out_data->skins[s].opaque_graphics_primitives = (vk_skeletal_graphics_primitive**) utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                                }
                                else
                                {
                                    out_data->skins[s].opaque_graphics_primitives = (vk_skeletal_graphics_primitive**) utils_realloc_zero (out_data->skins[s].opaque_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skins[s].opaque_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skins[s].opaque_graphics_primitives_count + 1));
                                }

                                out_data->skins[s].opaque_graphics_primitives[out_data->skins[s].opaque_graphics_primitives_count] = out_data->graphics_primitives + rgp;
                                ++out_data->skins[s].opaque_graphics_primitives_count;
                            }

                            if (strstr (current_primitive->material->name, "alpha") != NULL)
                            {
                                if (out_data->skins[s].alpha_graphics_primitives == NULL)
                                {
                                    out_data->skins[s].alpha_graphics_primitives = (vk_skeletal_graphics_primitive**) utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                                }
                                else
                                {
                                    out_data->skins[s].alpha_graphics_primitives = (vk_skeletal_graphics_primitive**) utils_realloc_zero (out_data->skins[s].alpha_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skins[s].alpha_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skins[s].alpha_graphics_primitives_count + 1));
                                }

                                out_data->skins[s].alpha_graphics_primitives[out_data->skins[s].alpha_graphics_primitives_count] = out_data->graphics_primitives + rgp;
                                ++out_data->skins[s].alpha_graphics_primitives_count;
                            }

                            if (strstr (current_primitive->material->name, "blend") != NULL)
                            {
                                if (out_data->skins[s].blend_graphics_primitives == NULL)
                                {
                                    out_data->skins[s].blend_graphics_primitives = (vk_skeletal_graphics_primitive**) utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                                }
                                else
                                {
                                    out_data->skins[s].blend_graphics_primitives = (vk_skeletal_graphics_primitive**) utils_realloc_zero (out_data->skins[s].blend_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skins[s].blend_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skins[s].blend_graphics_primitives_count + 1));
                                }

                                out_data->skins[s].blend_graphics_primitives[out_data->skins[s].blend_graphics_primitives_count] = out_data->graphics_primitives + rgp;
                                ++out_data->skins[s].blend_graphics_primitives_count;
                            }
                        }
                    }
                }
            }
        }
    }

    return AGAINST_SUCCESS;
}*/

AGAINST_RESULT link_animations_to_skins (scene_asset_data* out_data)
{
    OutputDebugString (L"link_animations_to_skins\n");

    for (size_t s = 0; s < out_data->skins_count; ++s)
    {
        vk_skin* current_skin = out_data->skins + s;
        for (size_t a = 0; a < out_data->animations_count; ++a)
        {
            vk_animation* current_anim = out_data->animations + a;

            if (strstr (current_anim->name, current_skin->name) != NULL)
            {
                if (current_skin->animations == NULL)
                {
                    current_skin->animations = (vk_animation**)utils_calloc (1, sizeof (vk_animation*));
                }
                else
                {
                    current_skin->animations = (vk_animation**)utils_realloc_zero (current_skin->animations, sizeof (vk_animation*) * out_data->animations_count, sizeof (vk_animation*) * (out_data->animations_count + 1));
                }

                current_skin->animations[current_skin->animations_count] = out_data->animations + a;
                ++current_skin->animations_count;
            }
        }
    }

    return AGAINST_SUCCESS;
}

/*AGAINST_RESULT link_skins_to_meshes (scene_asset_data* out_data)
{
    OutputDebugString (L"link_skins_to_meshes\n");

    for (size_t n = 0; n < ref_cgltf_mesh_nodes_count; ++n)
    {
        for (size_t s = 0; s < ref_cgltf_skins_count; ++s)
        {
            if (ref_cgltf_mesh_nodes[n]->skin == ref_cgltf_skins[s])
            {
                out_data->skeletal_meshes[n].skin = out_data->skins + s;
            }
        }
    }

    return AGAINST_SUCCESS;
}*/

AGAINST_RESULT link_graphics_primitives_to_skeletal_meshes (scene_asset_data* out_data)
{
    OutputDebugString (L"link_graphics_primitives_to_skeletal_meshes\n");

    for (size_t m = 0; m < ref_cgltf_mesh_nodes_count; ++m)
    {
        cgltf_node* current_node = ref_cgltf_mesh_nodes[m];
        for (size_t mgp = 0; mgp < current_node->mesh->primitives_count; ++mgp)
        {
            cgltf_primitive* current_mgp = current_node->mesh->primitives + mgp;
            for (size_t gp = 0; gp < ref_cgltf_graphics_primitives_count; ++gp)
            {
                cgltf_primitive* current_gp = ref_cgltf_graphics_primitives[gp];

                if (current_mgp == current_gp)
                {
                    if (strstr (current_mgp->material->name, "opaque") != NULL)
                    {
                        if (out_data->skeletal_meshes[m].opaque_graphics_primitives == NULL)
                        {
                            out_data->skeletal_meshes[m].opaque_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                        }
                        else
                        {
                            out_data->skeletal_meshes[m].opaque_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (out_data->skeletal_meshes[m].opaque_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skeletal_meshes[m].opaque_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skeletal_meshes[m].opaque_graphics_primitives_count + 1));
                        }

                        out_data->skeletal_meshes[m].opaque_graphics_primitives[out_data->skeletal_meshes[m].opaque_graphics_primitives_count] = out_data->graphics_primitives + gp;
                        ++out_data->skeletal_meshes[m].opaque_graphics_primitives_count;
                    }
                    
                    if (strstr (current_mgp->material->name, "alpha") != NULL)
                    {
                        if (out_data->skeletal_meshes[m].alpha_graphics_primitives == NULL)
                        {
                            out_data->skeletal_meshes[m].alpha_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                        }
                        else
                        {
                            out_data->skeletal_meshes[m].alpha_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (out_data->skeletal_meshes[m].alpha_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skeletal_meshes[m].alpha_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skeletal_meshes[m].alpha_graphics_primitives_count + 1));
                        }

                        out_data->skeletal_meshes[m].alpha_graphics_primitives[out_data->skeletal_meshes[m].alpha_graphics_primitives_count] = out_data->graphics_primitives + gp;
                        ++out_data->skeletal_meshes[m].alpha_graphics_primitives_count;
                    }

                    if (strstr (current_mgp->material->name, "blend") != NULL)
                    {
                        if (out_data->skeletal_meshes[m].blend_graphics_primitives == NULL)
                        {
                            out_data->skeletal_meshes[m].blend_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_calloc (1, sizeof (vk_skeletal_graphics_primitive*));
                        }
                        else
                        {
                            out_data->skeletal_meshes[m].blend_graphics_primitives = (vk_skeletal_graphics_primitive**)utils_realloc_zero (out_data->skeletal_meshes[m].blend_graphics_primitives, sizeof (vk_skeletal_graphics_primitive*) * out_data->skeletal_meshes[m].blend_graphics_primitives_count, sizeof (vk_skeletal_graphics_primitive*) * (out_data->skeletal_meshes[m].blend_graphics_primitives_count + 1));
                        }

                        out_data->skeletal_meshes[m].blend_graphics_primitives[out_data->skeletal_meshes[m].blend_graphics_primitives_count] = out_data->graphics_primitives + gp;
                        ++out_data->skeletal_meshes[m].blend_graphics_primitives_count;
                    }
                }
            }
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT link_skeletal_meshes_to_skins (scene_asset_data* out_data)
{
    OutputDebugString (L"link_skeletal_meshes_to_skins\n");

    for (size_t s = 0; s < ref_cgltf_skins_count; ++s)
    {
        cgltf_skin* current_skin = ref_cgltf_skins[s];
        for (size_t m = 0; m < ref_cgltf_mesh_nodes_count; ++m)
        {
            cgltf_node* current_node = ref_cgltf_mesh_nodes[m];
            if (current_skin == current_node->skin)
            {
                if (out_data->skins[s].skeletal_meshes == NULL)
                {
                    out_data->skins[s].skeletal_meshes = (vk_skeletal_mesh**)utils_calloc (1, sizeof (vk_skeletal_mesh*));
                }
                else
                {
                    out_data->skins[s].skeletal_meshes = (vk_skeletal_mesh**)utils_realloc_zero (out_data->skins[s].skeletal_meshes, sizeof (vk_skeletal_mesh*) * out_data->skins[s].skeletal_meshes_count, sizeof (vk_skeletal_mesh*) * (out_data->skins[s].skeletal_meshes_count + 1));
                }

                out_data->skins[s].skeletal_meshes[out_data->skins[s].skeletal_meshes_count] = out_data->skeletal_meshes + m;
                ++out_data->skins[s].skeletal_meshes_count;
            }
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_gltf_datas (const char* full_folder_path, cgltf_data** datas, size_t datas_count, scene_asset_data* out_data)
{
    OutputDebugString (L"import_gltf_datas\n");

    AGAINST_RESULT result;
    
    CHECK_AGAINST_RESULT (import_images (full_folder_path, datas, datas_count, out_data), result);
    CHECK_AGAINST_RESULT (import_materials (datas, datas_count, out_data), result);
    CHECK_AGAINST_RESULT (import_graphics_primitives (datas, datas_count, out_data), result);
    CHECK_AGAINST_RESULT (link_materials_to_graphics_primitives(out_data), result);
    CHECK_AGAINST_RESULT (import_skins (datas, datas_count, out_data), result);
    CHECK_AGAINST_RESULT (import_animations (datas, datas_count, out_data), result);
    CHECK_AGAINST_RESULT (link_animations_to_skins (out_data), result);
    CHECK_AGAINST_RESULT (import_skeletal_meshes (datas, datas_count, out_data), result);
    CHECK_AGAINST_RESULT (link_graphics_primitives_to_skeletal_meshes (out_data), result);
    CHECK_AGAINST_RESULT (link_skeletal_meshes_to_skins (out_data), result);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT gather_gltf_datas (const char* full_file_path, cgltf_data** datas, size_t current_data_index)
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
    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_gltf_files_from_folder (const char* partial_folder_path, scene_asset_data** out_gltf_data)
{
    OutputDebugString (L"import_gltf_files_from_folder\n");
    
    file_path* file_paths = NULL;
    size_t num_files = 0;
    utils_get_files_in_folder (partial_folder_path, &file_paths, &num_files);

    gltf_datas_count = num_files;
    gltf_datas = (cgltf_data**)utils_calloc (num_files, sizeof (cgltf_data*));

    char full_folder_path[MAX_PATH];
    utils_get_full_file_path (partial_folder_path, full_folder_path);

    AGAINST_RESULT result;

    for (size_t f = 0 ; f < num_files; ++f)
    {
        char current_full_file_path[MAX_PATH];
        strcpy (current_full_file_path, full_folder_path);
        strcat (current_full_file_path, file_paths[f].path);
        CHECK_AGAINST_RESULT (gather_gltf_datas (current_full_file_path, gltf_datas, f), result);
    }

    utils_free (file_paths);
    file_paths = NULL;

    *out_gltf_data = (scene_asset_data*)utils_calloc (1, sizeof (scene_asset_data));

    CHECK_AGAINST_RESULT (import_gltf_datas (full_folder_path, gltf_datas, gltf_datas_count, *out_gltf_data), result);

    for (size_t d = 0; d < gltf_datas_count; ++d)
    {
        cgltf_free (gltf_datas[d]);
    }

    utils_free (gltf_datas);
    gltf_datas = NULL;
    gltf_datas_count = 0;

    utils_free (ref_cgltf_images);
    ref_cgltf_images = NULL;
    ref_cgltf_images_count = 0;

    utils_free (ref_cgltf_materials);
    ref_cgltf_materials = NULL;
    ref_cgltf_materials_count = 0;

    utils_free (ref_cgltf_anims);
    ref_cgltf_anims = NULL;
    ref_cgltf_anims_count = 0;

    utils_free (ref_cgltf_graphics_primitives);
    ref_cgltf_graphics_primitives = NULL;
    ref_cgltf_graphics_primitives_count = 0;

    utils_free (ref_cgltf_joints);
    ref_cgltf_joints = NULL;
    ref_cgltf_joints_count = 0;

    utils_free (ref_cgltf_skins);
    ref_cgltf_skins = NULL;
    ref_cgltf_skins_count = 0;

    utils_free (ref_cgltf_mesh_nodes);
    ref_cgltf_mesh_nodes = NULL;
    ref_cgltf_mesh_nodes_count = 0;

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_gltf_file (const char* file_path)
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

    return AGAINST_SUCCESS;
}