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
size_t num_gltf_datas = 0;

cgltf_image** ref_cgltf_images = NULL;
size_t num_ref_cgltf_images = 0;

cgltf_material** ref_cgltf_materials = NULL;
size_t num_ref_cgltf_materials = 0;

cgltf_animation** ref_cgltf_anims = NULL;
size_t num_ref_cgltf_anims = 0;

cgltf_primitive** ref_cgltf_skinned_graphics_primitives = NULL;
size_t num_ref_cgltf_skinned_graphics_primitives = 0;

cgltf_primitive** ref_cgltf_static_graphics_primitives = NULL;
size_t num_ref_cgltf_static_graphics_primitives = 0;

cgltf_node** ref_cgltf_joints = NULL;
size_t num_ref_cgltf_joints = 0;

cgltf_skin** ref_cgltf_skins = NULL;
size_t num_ref_cgltf_skins = 0;

cgltf_node** ref_cgltf_skinned_mesh_nodes = NULL;
size_t num_ref_cgltf_skinned_mesh_nodes = 0;

cgltf_node** ref_cgltf_static_mesh_nodes = NULL;
size_t num_ref_cgltf_static_mesh_nodes = 0;

typedef struct _joint_data
{
    float* translations;
    size_t num_translations;

    float* rotations;
    size_t num_rotations;

    float* matrices;
    size_t num_matrices;
    
    char joint_name[2048];
    char anim_name[2048];
    char skin_name[2048];
} joint_data;

typedef struct _anim_joint_data
{
    joint_data* joint_anims;
    size_t num_joint_anims;
    
    size_t num_frames;

    char name[2048];
} anim_joint_data;

AGAINST_RESULT import_images (const char* full_folder_path, cgltf_data** datas, size_t num_datas, scene_graphics* out_data)
{
    OutputDebugString (L"import_images\n");

    size_t total_image_size = 0;

    size_t* img_offsets = NULL;
    size_t* img_sizes = NULL;
    size_t* img_widths = NULL;
    size_t* img_heights = NULL;
    uint8_t** img_pixels = NULL;

    AGAINST_RESULT result = AGAINST_SUCCESS;

    size_t current_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        if (out_data->images == NULL)
        {
            out_data->images = (VkImage*)utils_calloc (current_data->images_count, sizeof (VkImage));
        }
        else
        {
            out_data->images = (VkImage*)utils_realloc_zero (out_data->images, sizeof (VkImage) * out_data->num_images, sizeof (VkImage) * (out_data->num_images + current_data->images_count));
        }

        if (out_data->image_views == NULL)
        {
            out_data->image_views = (VkImageView*)utils_calloc (current_data->images_count, sizeof (VkImageView));
        }
        else
        {
            out_data->image_views = (VkImageView*)utils_realloc_zero (out_data->image_views, sizeof (VkImageView) * out_data->num_images, sizeof (VkImageView) * (out_data->num_images + current_data->images_count));
        }

        if (img_offsets == NULL)
        {
            img_offsets = (size_t*)utils_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_offsets = (size_t*)utils_realloc_zero (img_offsets, sizeof (size_t) * out_data->num_images, sizeof (size_t) * (out_data->num_images + current_data->images_count));
        }

        if (img_sizes == NULL)
        {
            img_sizes = (size_t*)utils_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_sizes = (size_t*)utils_realloc_zero (img_sizes, sizeof (size_t) * out_data->num_images, sizeof (size_t) * (out_data->num_images + current_data->images_count));
        }

        if (img_widths == NULL)
        {
            img_widths = (size_t*)utils_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_widths = (size_t*)utils_realloc_zero (img_widths, sizeof (size_t) * out_data->num_images, sizeof (size_t) * (out_data->num_images + current_data->images_count));
        }

        if (img_heights == NULL)
        {
            img_heights = (size_t*)utils_calloc (current_data->images_count, sizeof (size_t));
        }
        else
        {
            img_heights = (size_t*)utils_realloc_zero (img_heights, sizeof (size_t) * out_data->num_images, sizeof (size_t) * (out_data->num_images + current_data->images_count));
        }

        if (img_pixels == NULL)
        {
            img_pixels = (uint8_t**)utils_calloc (current_data->images_count, sizeof (uint8_t*));
        }
        else
        {
            img_pixels = (uint8_t**)utils_realloc_zero (img_pixels, sizeof (uint8_t*) * out_data->num_images, sizeof (uint8_t*) * (out_data->num_images + current_data->images_count));
        }

        if (ref_cgltf_images == NULL)
        {
            ref_cgltf_images = (cgltf_image**)utils_calloc (current_data->images_count, sizeof (cgltf_image*));
        }
        else
        {
            ref_cgltf_images = (cgltf_image**)utils_realloc_zero (ref_cgltf_images, sizeof (cgltf_image*) * out_data->num_images , sizeof (cgltf_image*) * (out_data->num_images + current_data->images_count));
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

        num_ref_cgltf_images += current_data->images_count;
        out_data->num_images += current_data->images_count;
    }

    VkBuffer staging_buffer; VkDeviceMemory staging_memory;
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (graphics_device, total_image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, transfer_queue_family_index, &staging_buffer), result);
    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (graphics_device, &staging_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_memory), result);

    for (size_t i = 0; i < num_ref_cgltf_images; ++i)
    {
        CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (graphics_device, staging_memory, img_offsets[i], img_sizes[i], img_pixels[i]), result);
    }

    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_image_memory (graphics_device, out_data->images, num_ref_cgltf_images, physical_device_memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &out_data->images_memory), result);

    for (size_t i = 0; i < num_ref_cgltf_images; ++i)
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
    
    for (size_t i = 0; i < num_ref_cgltf_images; ++i)
    {
        stbi_image_free (img_pixels[i]);
    }
    
    utils_free (img_pixels);

    vk_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_memory);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_materials (cgltf_data** datas, size_t num_datas, scene_graphics* out_data)
{
    OutputDebugString (L"import_materials\n");

    size_t current_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        if (out_data->materials == NULL)
        {
            out_data->materials = (vk_material*)utils_calloc (current_data->materials_count, sizeof (vk_material));
        }
        else
        {
            out_data->materials = (vk_material*)utils_realloc_zero (out_data->materials, sizeof (vk_material) * out_data->num_materials, sizeof (vk_material) * (current_data->materials_count + out_data->num_materials));
        }

        if (ref_cgltf_materials == NULL)
        {
            ref_cgltf_materials = (cgltf_material**)utils_calloc (current_data->materials_count, sizeof (cgltf_material*));
        }
        else
        {
            ref_cgltf_materials = (cgltf_material**)utils_realloc_zero (ref_cgltf_materials, sizeof (cgltf_material*) * out_data->num_materials, sizeof (cgltf_material*) * (out_data->num_materials + current_data->materials_count));
        }

        for (size_t m = 0; m < current_data->materials_count; ++m)
        {
            cgltf_material* material = current_data->materials + m;

            strcpy (out_data->materials[current_index].name, material->name);

            for (size_t i = 0; i < num_ref_cgltf_images; ++i)
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

        out_data->num_materials += current_data->materials_count;
        num_ref_cgltf_materials += current_data->materials_count;
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_graphics_primitives (cgltf_data** datas, size_t num_datas, scene_graphics* out_data)
{
    OutputDebugString (L"import_graphics_primitives\n");
    unsigned char** skinned_positions = NULL;
    unsigned char** skinned_normals = NULL;
    unsigned char** skinned_uv0s = NULL;
    unsigned char** skinned_uv1s = NULL;
    unsigned char** skinned_joints = NULL;
    unsigned char** skinned_weights = NULL;
    unsigned char** skinned_indices = NULL;

    size_t* skinned_positions_sizes = NULL;
    size_t* skinned_positions_offsets = NULL;
    size_t* skinned_normals_sizes = NULL;
    size_t* skinned_normals_offsets = NULL;
    size_t* skinned_uv0s_sizes = NULL;
    size_t* skinned_uv0s_offsets = NULL;
    size_t* skinned_uv1s_sizes = NULL;
    size_t* skinned_uv1s_offsets = NULL;
    size_t* skinned_joints_sizes = NULL;
    size_t* skinned_joints_offsets = NULL;
    size_t* skinned_weights_sizes = NULL;
    size_t* skinned_weights_offsets = NULL;
    size_t* skinned_nums_indices = NULL;
    size_t* skinned_indices_sizes = NULL;
    size_t* skinned_indices_offsets = NULL;
    VkIndexType* skinned_indices_types = NULL;

    unsigned char** static_positions = NULL;
    unsigned char** static_normals = NULL;
    unsigned char** static_uv0s = NULL;
    unsigned char** static_uv1s = NULL;
    unsigned char** static_indices = NULL;

    size_t* static_positions_sizes = NULL;
    size_t* static_positions_offsets = NULL;
    size_t* static_normals_sizes = NULL;
    size_t* static_normals_offsets = NULL;
    size_t* static_uv0s_sizes = NULL;
    size_t* static_uv0s_offsets = NULL;
    size_t* static_uv1s_sizes = NULL;
    size_t* static_uv1s_offsets = NULL;
    size_t* static_nums_indices = NULL;
    size_t* static_indices_sizes = NULL;
    size_t* static_indices_offsets = NULL;
    VkIndexType* static_indices_types = NULL;

    size_t current_primitive_data_offset = 0;
    
    size_t num_skinned_primitives_data = 0;
    size_t current_skinned_primitive_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
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

            if (current_node->skin == NULL)
            {
                continue;
            }

            cgltf_mesh* current_mesh = current_node->mesh;

            if (skinned_positions == NULL)
            {
                skinned_positions = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                skinned_positions = (unsigned char**)utils_realloc_zero (skinned_positions, sizeof (unsigned char*) * num_skinned_primitives_data, sizeof (unsigned char*) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_positions_sizes == NULL)
            {
                skinned_positions_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_positions_sizes = (size_t*)utils_realloc_zero (skinned_positions_sizes, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_positions_offsets == NULL)
            {
                skinned_positions_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_positions_offsets = (size_t*)utils_realloc_zero (skinned_positions_offsets, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_normals == NULL)
            {
                skinned_normals = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                skinned_normals = (unsigned char**)utils_realloc_zero (skinned_normals, sizeof (unsigned char*) * num_skinned_primitives_data, sizeof (unsigned char*) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_normals_sizes == NULL)
            {
                skinned_normals_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_normals_sizes = (size_t*)utils_realloc_zero (skinned_normals_sizes, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_normals_offsets == NULL)
            {
                skinned_normals_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_normals_offsets = (size_t*)utils_realloc_zero (skinned_normals_offsets, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_uv0s == NULL)
            {
                skinned_uv0s = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                skinned_uv0s = (unsigned char**)utils_realloc_zero (skinned_uv0s, sizeof (unsigned char*) * num_skinned_primitives_data, sizeof (unsigned char*) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_uv0s_sizes == NULL)
            {
                skinned_uv0s_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_uv0s_sizes = (size_t*)utils_realloc_zero (skinned_uv0s_sizes, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_uv0s_offsets == NULL)
            {
                skinned_uv0s_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_uv0s_offsets = (size_t*)utils_realloc_zero (skinned_uv0s_offsets, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_uv1s == NULL)
            {
                skinned_uv1s = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                skinned_uv1s = (unsigned char**)utils_realloc_zero (skinned_uv1s, sizeof (unsigned char*) * num_skinned_primitives_data , sizeof (unsigned char*) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_uv1s_sizes == NULL)
            {
                skinned_uv1s_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_uv1s_sizes = (size_t*)utils_realloc_zero (skinned_uv1s_sizes, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_uv1s_offsets == NULL)
            {
                skinned_uv1s_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_uv1s_offsets = (size_t*)utils_realloc_zero (skinned_uv1s_offsets, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_joints == NULL)
            {
                skinned_joints = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                skinned_joints = (unsigned char**)utils_realloc_zero (skinned_joints, sizeof (unsigned char*) * num_skinned_primitives_data, sizeof (unsigned char*) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_joints_sizes == NULL)
            {
                skinned_joints_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_joints_sizes = (size_t*)utils_realloc_zero (skinned_joints_sizes, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_joints_offsets == NULL)
            {
                skinned_joints_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_joints_offsets = (size_t*)utils_realloc_zero (skinned_joints_offsets, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_weights == NULL)
            {
                skinned_weights = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                skinned_weights = (unsigned char**)utils_realloc_zero (skinned_weights,  sizeof (unsigned char*) * num_skinned_primitives_data, sizeof (unsigned char*) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_weights_sizes == NULL)
            {
                skinned_weights_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_weights_sizes = (size_t*)utils_realloc_zero (skinned_weights_sizes, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_weights_offsets == NULL)
            {
                skinned_weights_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_weights_offsets = (size_t*)utils_realloc_zero (skinned_weights_offsets, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_indices == NULL)
            {
                skinned_indices = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                skinned_indices = (unsigned char**)utils_realloc_zero (skinned_indices, sizeof (unsigned char*) * num_skinned_primitives_data, sizeof (unsigned char*) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_nums_indices == NULL)
            {
                skinned_nums_indices = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_nums_indices = (size_t*)utils_realloc_zero (skinned_nums_indices, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_indices_sizes == NULL)
            {
                skinned_indices_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_indices_sizes = (size_t*)utils_realloc_zero (skinned_indices_sizes, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_indices_offsets == NULL)
            {
                skinned_indices_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                skinned_indices_offsets = (size_t*)utils_realloc_zero (skinned_indices_offsets, sizeof (size_t) * num_skinned_primitives_data, sizeof (size_t) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (skinned_indices_types == NULL)
            {
                skinned_indices_types = (VkIndexType*)utils_calloc (current_mesh->primitives_count, sizeof (VkIndexType));
            }
            else
            {
                skinned_indices_types = (VkIndexType*)utils_realloc_zero (skinned_indices_types, sizeof (VkIndexType) * num_skinned_primitives_data, sizeof (VkIndexType) * (num_skinned_primitives_data + current_mesh->primitives_count));
            }

            if (ref_cgltf_skinned_graphics_primitives == NULL)
            {
                ref_cgltf_skinned_graphics_primitives = (cgltf_primitive**)utils_calloc (current_mesh->primitives_count, sizeof (cgltf_primitive*));
            }
            else
            {
                ref_cgltf_skinned_graphics_primitives = (cgltf_primitive**)utils_realloc_zero (ref_cgltf_skinned_graphics_primitives, sizeof (cgltf_primitive*) * num_skinned_primitives_data, sizeof (cgltf_primitive*) * (num_skinned_primitives_data + current_mesh->primitives_count));
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
                        skinned_positions[current_skinned_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        skinned_positions_sizes[current_skinned_primitive_index] = buffer_view->size;
                        skinned_positions_offsets[current_skinned_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                    else if (current_attribute->type == cgltf_attribute_type_normal)
                    {
                        skinned_normals[current_skinned_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        skinned_normals_sizes[current_skinned_primitive_index] = buffer_view->size;
                        skinned_normals_offsets[current_skinned_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                    else if (current_attribute->type == cgltf_attribute_type_texcoord)
                    {
                        if (strcmp (current_attribute->name, "TEXCOORD_0") == 0)
                        {
                            skinned_uv0s[current_skinned_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                            skinned_uv0s_sizes[current_skinned_primitive_index] = buffer_view->size;
                            skinned_uv0s_offsets[current_skinned_primitive_index] = current_primitive_data_offset;
                            
                            current_primitive_data_offset += buffer_view->size;
                        }
                        else if (strcmp (current_attribute->name, "TEXCOORD_1") == 0)
                        {
                            skinned_uv1s[current_skinned_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                            skinned_uv1s_sizes[current_skinned_primitive_index] = buffer_view->size;
                            skinned_uv1s_offsets[current_skinned_primitive_index] = current_primitive_data_offset;

                            current_primitive_data_offset += buffer_view->size;
                        }
                    }
                    else if (current_attribute->type == cgltf_attribute_type_joints)
                    {
                        skinned_joints[current_skinned_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        skinned_joints_sizes[current_skinned_primitive_index] = buffer_view->size;
                        skinned_joints_offsets[current_skinned_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                    else if (current_attribute->type == cgltf_attribute_type_weights)
                    {
                        skinned_weights[current_skinned_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        skinned_weights_sizes[current_skinned_primitive_index] = buffer_view->size;
                        skinned_weights_offsets[current_skinned_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                }

                cgltf_accessor* accessor = current_primitive->indices;
                cgltf_buffer_view* buffer_view = accessor->buffer_view;
                skinned_indices[current_skinned_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                skinned_nums_indices[current_skinned_primitive_index] = accessor->count;
                skinned_indices_sizes[current_skinned_primitive_index] = buffer_view->size;
                skinned_indices_offsets[current_skinned_primitive_index] = current_primitive_data_offset;

                switch (accessor->component_type)
                {
                case cgltf_component_type_r_8:
                    skinned_indices_types[current_skinned_primitive_index] = VK_INDEX_TYPE_UINT8_EXT;
                    break;

                case cgltf_component_type_r_16u:
                    skinned_indices_types[current_skinned_primitive_index] = VK_INDEX_TYPE_UINT16;
                    break;

                case cgltf_component_type_r_32u:
                    skinned_indices_types[current_skinned_primitive_index] = VK_INDEX_TYPE_UINT32;
                    break;

                default:
                    break;
                }

                current_primitive_data_offset += buffer_view->size;

                ref_cgltf_skinned_graphics_primitives[current_skinned_primitive_index] = current_primitive;
                
                ++current_skinned_primitive_index;
            }

            num_skinned_primitives_data += current_mesh->primitives_count;
            num_ref_cgltf_skinned_graphics_primitives += current_mesh->primitives_count;
        }
    }

    size_t num_static_primitives_data = 0;
    size_t current_static_primitive_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
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

            if (current_node->skin != NULL)
            {
                continue;
            }

            cgltf_mesh* current_mesh = current_node->mesh;

            if (static_positions == NULL)
            {
                static_positions = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                static_positions = (unsigned char**)utils_realloc_zero (static_positions, sizeof (unsigned char*) * num_static_primitives_data, sizeof (unsigned char*) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_positions_sizes == NULL)
            {
                static_positions_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_positions_sizes = (size_t*)utils_realloc_zero (static_positions_sizes, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_positions_offsets == NULL)
            {
                static_positions_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_positions_offsets = (size_t*)utils_realloc_zero (static_positions_offsets, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_normals == NULL)
            {
                static_normals = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                static_normals = (unsigned char**)utils_realloc_zero (static_normals, sizeof (unsigned char*) * num_static_primitives_data, sizeof (unsigned char*) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_normals_sizes == NULL)
            {
                static_normals_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_normals_sizes = (size_t*)utils_realloc_zero (static_normals_sizes, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_normals_offsets == NULL)
            {
                static_normals_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_normals_offsets = (size_t*)utils_realloc_zero (static_normals_offsets, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_uv0s == NULL)
            {
                static_uv0s = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                static_uv0s = (unsigned char**)utils_realloc_zero (static_uv0s, sizeof (unsigned char*) * num_static_primitives_data, sizeof (unsigned char*) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_uv0s_sizes == NULL)
            {
                static_uv0s_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_uv0s_sizes = (size_t*)utils_realloc_zero (static_uv0s_sizes, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_uv0s_offsets == NULL)
            {
                static_uv0s_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_uv0s_offsets = (size_t*)utils_realloc_zero (static_uv0s_offsets, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_uv1s == NULL)
            {
                static_uv1s = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                static_uv1s = (unsigned char**)utils_realloc_zero (static_uv1s, sizeof (unsigned char*) * num_static_primitives_data, sizeof (unsigned char*) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_uv1s_sizes == NULL)
            {
                static_uv1s_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_uv1s_sizes = (size_t*)utils_realloc_zero (static_uv1s_sizes, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_uv1s_offsets == NULL)
            {
                static_uv1s_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_uv1s_offsets = (size_t*)utils_realloc_zero (static_uv1s_offsets, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_indices == NULL)
            {
                static_indices = (unsigned char**)utils_calloc (current_mesh->primitives_count, sizeof (unsigned char*));
            }
            else
            {
                static_indices = (unsigned char**)utils_realloc_zero (static_indices, sizeof (unsigned char*) * num_static_primitives_data, sizeof (unsigned char*) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_nums_indices == NULL)
            {
                static_nums_indices = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_nums_indices = (size_t*)utils_realloc_zero (static_nums_indices, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_indices_sizes == NULL)
            {
                static_indices_sizes = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_indices_sizes = (size_t*)utils_realloc_zero (static_indices_sizes, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_indices_offsets == NULL)
            {
                static_indices_offsets = (size_t*)utils_calloc (current_mesh->primitives_count, sizeof (size_t));
            }
            else
            {
                static_indices_offsets = (size_t*)utils_realloc_zero (static_indices_offsets, sizeof (size_t) * num_static_primitives_data, sizeof (size_t) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (static_indices_types == NULL)
            {
                static_indices_types = (VkIndexType*)utils_calloc (current_mesh->primitives_count, sizeof (VkIndexType));
            }
            else
            {
                static_indices_types = (VkIndexType*)utils_realloc_zero (static_indices_types, sizeof (VkIndexType) * num_static_primitives_data, sizeof (VkIndexType) * (num_static_primitives_data + current_mesh->primitives_count));
            }

            if (ref_cgltf_static_graphics_primitives == NULL)
            {
                ref_cgltf_static_graphics_primitives = (cgltf_primitive**)utils_calloc (current_mesh->primitives_count, sizeof (cgltf_primitive*));
            }
            else
            {
                ref_cgltf_static_graphics_primitives = (cgltf_primitive**)utils_realloc_zero (ref_cgltf_static_graphics_primitives, sizeof (cgltf_primitive*) * num_static_primitives_data, sizeof (cgltf_primitive*) * (num_static_primitives_data + current_mesh->primitives_count));
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
                        static_positions[current_static_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        static_positions_sizes[current_static_primitive_index] = buffer_view->size;
                        static_positions_offsets[current_static_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                    else if (current_attribute->type == cgltf_attribute_type_normal)
                    {
                        static_normals[current_static_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                        static_normals_sizes[current_static_primitive_index] = buffer_view->size;
                        static_normals_offsets[current_static_primitive_index] = current_primitive_data_offset;

                        current_primitive_data_offset += buffer_view->size;
                    }
                    else if (current_attribute->type == cgltf_attribute_type_texcoord)
                    {
                        if (strcmp (current_attribute->name, "TEXCOORD_0") == 0)
                        {
                            static_uv0s[current_static_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                            static_uv0s_sizes[current_static_primitive_index] = buffer_view->size;
                            static_uv0s_offsets[current_static_primitive_index] = current_primitive_data_offset;

                            current_primitive_data_offset += buffer_view->size;
                        }
                        else if (strcmp (current_attribute->name, "TEXCOORD_1") == 0)
                        {
                            static_uv1s[current_static_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                            static_uv1s_sizes[current_static_primitive_index] = buffer_view->size;
                            static_uv1s_offsets[current_static_primitive_index] = current_primitive_data_offset;

                            current_primitive_data_offset += buffer_view->size;
                        }
                    }
                }

                cgltf_accessor* accessor = current_primitive->indices;
                cgltf_buffer_view* buffer_view = accessor->buffer_view;
                static_indices[current_static_primitive_index] = (unsigned char*)buffer_view->buffer->data + accessor->offset + buffer_view->offset;
                static_nums_indices[current_static_primitive_index] = accessor->count;
                static_indices_sizes[current_static_primitive_index] = buffer_view->size;
                static_indices_offsets[current_static_primitive_index] = current_primitive_data_offset;

                switch (accessor->component_type)
                {
                case cgltf_component_type_r_8:
                    static_indices_types[current_static_primitive_index] = VK_INDEX_TYPE_UINT8_EXT;
                    break;

                case cgltf_component_type_r_16u:
                    static_indices_types[current_static_primitive_index] = VK_INDEX_TYPE_UINT16;
                    break;

                case cgltf_component_type_r_32u:
                    static_indices_types[current_static_primitive_index] = VK_INDEX_TYPE_UINT32;
                    break;

                default:
                    break;
                }

                current_primitive_data_offset += buffer_view->size;

                ref_cgltf_static_graphics_primitives[current_static_primitive_index] = current_primitive;

                ++current_static_primitive_index;
            }

            num_static_primitives_data += current_mesh->primitives_count;
            num_ref_cgltf_static_graphics_primitives += current_mesh->primitives_count;
        }
    }
    
    AGAINST_RESULT result = AGAINST_SUCCESS;
    
    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
   
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (
        graphics_device, 
        current_primitive_data_offset, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_SHARING_MODE_EXCLUSIVE, 
        transfer_queue_family_index, 
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

    out_data->skinned_graphics_primitives = (vk_skinned_primitive*)utils_calloc (num_skinned_primitives_data, sizeof (vk_skinned_primitive));
    out_data->num_skinned_graphics_primitives = num_skinned_primitives_data;
    out_data->static_graphics_primitives = (vk_static_primitive*)utils_calloc (num_static_primitives_data, sizeof (vk_static_primitive));
    out_data->num_static_graphics_primitives = num_static_primitives_data;

    for (size_t p = 0; p < num_skinned_primitives_data; ++p)
    {
        if (skinned_positions_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                skinned_positions_offsets[p],
                skinned_positions_sizes[p],
                skinned_positions[p]),
                result);
        }

        if (skinned_normals_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                skinned_normals_offsets[p],
                skinned_normals_sizes[p],
                skinned_normals[p]),
                result);
        }

        if (skinned_uv0s_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                skinned_uv0s_offsets[p],
                skinned_uv0s_sizes[p],
                skinned_uv0s[p]),
                result);
        }

        if (skinned_uv1s_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                skinned_uv1s_offsets[p],
                skinned_uv1s_sizes[p],
                skinned_uv1s[p]),
                result);
        }

        if (skinned_joints_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                skinned_joints_offsets[p],
                skinned_joints_sizes[p],
                skinned_joints[p]),
                result);
        }

        if (skinned_weights_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                skinned_weights_offsets[p],
                skinned_weights_sizes[p],
                skinned_weights[p]),
                result);
        }

        if (skinned_indices_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                skinned_indices_offsets[p],
                skinned_indices_sizes[p],
                skinned_indices[p]),
                result);
        }

        out_data->skinned_graphics_primitives[p].positions_offset = skinned_positions_offsets[p];
        out_data->skinned_graphics_primitives[p].normals_offset = skinned_normals_offsets[p];
        out_data->skinned_graphics_primitives[p].uv0s_offset = skinned_uv0s_offsets[p];
        out_data->skinned_graphics_primitives[p].uv1s_offset = skinned_uv1s_offsets[p];
        out_data->skinned_graphics_primitives[p].joints_offset = skinned_joints_offsets[p];
        out_data->skinned_graphics_primitives[p].weights_offset = skinned_weights_offsets[p];
        out_data->skinned_graphics_primitives[p].index_type = skinned_indices_types[p];
        out_data->skinned_graphics_primitives[p].num_indices = skinned_nums_indices[p];
        out_data->skinned_graphics_primitives[p].indices_offset = skinned_indices_offsets[p];
    }

    for (size_t p = 0; p < num_static_primitives_data; ++p)
    {
        if (static_positions_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                static_positions_offsets[p],
                static_positions_sizes[p],
                static_positions[p]),
                result);
        }

        if (static_normals_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                static_normals_offsets[p],
                static_normals_sizes[p],
                static_normals[p]),
                result);
        }

        if (static_uv0s_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                static_uv0s_offsets[p],
                static_uv0s_sizes[p],
                static_uv0s[p]),
                result);
        }

        if (static_uv1s_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                static_uv1s_offsets[p],
                static_uv1s_sizes[p],
                static_uv1s[p]),
                result);
        }

        if (static_indices_sizes[p] > 0)
        {
            CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (
                graphics_device,
                staging_buffer_memory,
                static_indices_offsets[p],
                static_indices_sizes[p],
                static_indices[p]),
                result);
        }

        out_data->static_graphics_primitives[p].positions_offset = static_positions_offsets[p];
        out_data->static_graphics_primitives[p].normals_offset = static_normals_offsets[p];
        out_data->static_graphics_primitives[p].uv0s_offset = static_uv0s_offsets[p];
        out_data->static_graphics_primitives[p].uv1s_offset = static_uv1s_offsets[p];
        out_data->static_graphics_primitives[p].index_type = static_indices_types[p];
        out_data->static_graphics_primitives[p].num_indices = static_nums_indices[p];
        out_data->static_graphics_primitives[p].indices_offset = static_indices_offsets[p];
    }

    CHECK_AGAINST_RESULT (
        vk_utils_create_buffer (
        graphics_device,
        current_primitive_data_offset,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_SHARING_MODE_EXCLUSIVE,
        transfer_queue_family_index,
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

    CHECK_AGAINST_RESULT (
        vk_utils_transfer_buffer_ownership (
            graphics_device,
            transfer_command_pool,
            transfer_queue,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_TRANSFER_BIT,
            VK_ACCESS_MEMORY_WRITE_BIT,
            VK_ACCESS_TRANSFER_WRITE_BIT,
            transfer_queue_family_index,
            graphics_queue_family_index,
            out_data->vb_ib,
            0,
            current_primitive_data_offset
        ),
    result
    );

    vk_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_buffer_memory);

    utils_free (skinned_positions);
    utils_free (skinned_positions_sizes);
    utils_free (skinned_positions_offsets);
    utils_free (skinned_normals);
    utils_free (skinned_normals_sizes);
    utils_free (skinned_normals_offsets);
    utils_free (skinned_uv0s);
    utils_free (skinned_uv0s_sizes);
    utils_free (skinned_uv0s_offsets);
    utils_free (skinned_uv1s);
    utils_free (skinned_uv1s_sizes);
    utils_free (skinned_uv1s_offsets);
    utils_free (skinned_joints);
    utils_free (skinned_joints_sizes);
    utils_free (skinned_joints_offsets);
    utils_free (skinned_weights);
    utils_free (skinned_weights_sizes);
    utils_free (skinned_weights_offsets);
    utils_free (skinned_indices);
    utils_free (skinned_nums_indices);
    utils_free (skinned_indices_sizes);
    utils_free (skinned_indices_offsets);
    utils_free (skinned_indices_types);
       
    utils_free (static_positions);
    utils_free (static_positions_sizes);
    utils_free (static_positions_offsets);
    utils_free (static_normals);
    utils_free (static_normals_sizes);
    utils_free (static_normals_offsets);
    utils_free (static_uv0s);
    utils_free (static_uv0s_sizes);
    utils_free (static_uv0s_offsets);
    utils_free (static_uv1s);
    utils_free (static_uv1s_sizes);
    utils_free (static_uv1s_offsets);
    utils_free (static_indices);
    utils_free (static_nums_indices);
    utils_free (static_indices_sizes);
    utils_free (static_indices_offsets);
    utils_free (static_indices_types);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT link_materials_to_graphics_primitives (scene_graphics* out_data)
{
    OutputDebugString (L"link_materials_to_graphics_primitives\n");

    for (size_t gp = 0; gp < num_ref_cgltf_skinned_graphics_primitives; ++gp)
    {
        cgltf_primitive* current_primitive = ref_cgltf_skinned_graphics_primitives[gp];

        for (size_t m = 0; m < num_ref_cgltf_materials; ++m)
        {
            cgltf_material* current_material = ref_cgltf_materials[m];

            if (current_primitive->material == current_material)
            {
                out_data->skinned_graphics_primitives[gp].material = out_data->materials + m;
            }
        }
    }

    for (size_t gp = 0; gp < num_ref_cgltf_static_graphics_primitives; ++gp)
    {
        cgltf_primitive* current_primitive = ref_cgltf_static_graphics_primitives[gp];

        for (size_t m = 0; m < num_ref_cgltf_materials; ++m)
        {
            cgltf_material* current_material = ref_cgltf_materials[m];

            if (current_primitive->material == current_material)
            {
                out_data->static_graphics_primitives[gp].material = out_data->materials + m;
            }
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_meshes (cgltf_data** datas, size_t num_datas, scene_graphics* out_data)
{
    OutputDebugString (L"import_meshes\n");

    size_t current_mesh_node_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
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

            if (current_node->skin == NULL)
            {
                continue;
            }

            if (out_data->skinned_meshes == NULL)
            {
                out_data->skinned_meshes = (vk_skinned_mesh*)utils_calloc (1, sizeof (vk_skinned_mesh));
            }
            else
            {
                out_data->skinned_meshes = (vk_skinned_mesh*)utils_realloc_zero (out_data->skinned_meshes, sizeof (vk_skinned_mesh) * out_data->num_skinned_meshes, sizeof (vk_skinned_mesh) * (out_data->num_skinned_meshes + 1));
            }

            if (ref_cgltf_skinned_mesh_nodes == NULL)
            {
                ref_cgltf_skinned_mesh_nodes = (cgltf_node**)utils_calloc (1, sizeof (cgltf_node*));
            }
            else
            {
                ref_cgltf_skinned_mesh_nodes = (cgltf_node**)utils_realloc_zero (ref_cgltf_skinned_mesh_nodes, sizeof (cgltf_node*) * out_data->num_skinned_meshes, sizeof (cgltf_node*) * (out_data->num_skinned_meshes + 1));
            }

            strcpy (out_data->skinned_meshes[current_mesh_node_index].name, current_node->name);

            ref_cgltf_skinned_mesh_nodes[current_mesh_node_index] = current_node;
            ++num_ref_cgltf_skinned_mesh_nodes;
            ++out_data->num_skinned_meshes;
            ++current_mesh_node_index;
        }
    }

    current_mesh_node_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
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

            if (current_node->skin != NULL)
            {
                continue;
            }

            if (out_data->static_meshes == NULL)
            {
                out_data->static_meshes = (vk_static_mesh*)utils_calloc (1, sizeof (vk_static_mesh));
            }
            else
            {
                out_data->static_meshes = (vk_static_mesh*)utils_realloc_zero (out_data->static_meshes, sizeof (vk_static_mesh) * out_data->num_static_meshes, sizeof (vk_static_mesh) * (out_data->num_static_meshes + 1));
            }

            if (ref_cgltf_static_mesh_nodes == NULL)
            {
                ref_cgltf_static_mesh_nodes = (cgltf_node**)utils_calloc (1, sizeof (cgltf_node*));
            }
            else
            {
                ref_cgltf_static_mesh_nodes = (cgltf_node**)utils_realloc_zero (ref_cgltf_static_mesh_nodes, sizeof (cgltf_node*) * out_data->num_static_meshes, sizeof (cgltf_node*) * (out_data->num_static_meshes + 1));
            }

            strcpy (out_data->static_meshes[current_mesh_node_index].name, current_node->name);

            ref_cgltf_static_mesh_nodes[current_mesh_node_index] = current_node;
            ++num_ref_cgltf_static_mesh_nodes;
            ++out_data->num_static_meshes;
            ++current_mesh_node_index;
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT link_graphics_primitives_to_meshes (scene_graphics* out_data)
{
    OutputDebugString (L"link_graphics_primitives_to_meshes\n");

    for (size_t m = 0; m < num_ref_cgltf_skinned_mesh_nodes; ++m)
    {
        cgltf_node* current_node = ref_cgltf_skinned_mesh_nodes[m];
        for (size_t mgp = 0; mgp < current_node->mesh->primitives_count; ++mgp)
        {
            cgltf_primitive* current_mgp = current_node->mesh->primitives + mgp;
            for (size_t gp = 0; gp < num_ref_cgltf_skinned_graphics_primitives; ++gp)
            {
                cgltf_primitive* current_gp = ref_cgltf_skinned_graphics_primitives[gp];

                if (current_mgp == current_gp)
                {
                    if (strstr (current_mgp->material->name, "opaque") != NULL)
                    {
                        if (out_data->skinned_meshes[m].opaque_graphics_primitives == NULL)
                        {
                            out_data->skinned_meshes[m].opaque_graphics_primitives = (vk_skinned_primitive**)utils_calloc (1, sizeof (vk_skinned_primitive*));
                        }
                        else
                        {
                            out_data->skinned_meshes[m].opaque_graphics_primitives = (vk_skinned_primitive**)utils_realloc_zero (out_data->skinned_meshes[m].opaque_graphics_primitives, sizeof (vk_skinned_primitive*) * out_data->skinned_meshes[m].num_opaque_graphics_primitives, sizeof (vk_skinned_primitive*) * (out_data->skinned_meshes[m].num_opaque_graphics_primitives + 1));
                        }

                        out_data->skinned_meshes[m].opaque_graphics_primitives[out_data->skinned_meshes[m].num_opaque_graphics_primitives] = out_data->skinned_graphics_primitives + gp;
                        ++out_data->skinned_meshes[m].num_opaque_graphics_primitives;
                    }
                    
                    if (strstr (current_mgp->material->name, "alpha") != NULL)
                    {
                        if (out_data->skinned_meshes[m].alpha_graphics_primitives == NULL)
                        {
                            out_data->skinned_meshes[m].alpha_graphics_primitives = (vk_skinned_primitive**)utils_calloc (1, sizeof (vk_skinned_primitive*));
                        }
                        else
                        {
                            out_data->skinned_meshes[m].alpha_graphics_primitives = (vk_skinned_primitive**)utils_realloc_zero (out_data->skinned_meshes[m].alpha_graphics_primitives, sizeof (vk_skinned_primitive*) * out_data->skinned_meshes[m].num_alpha_graphics_primitives, sizeof (vk_skinned_primitive*) * (out_data->skinned_meshes[m].num_alpha_graphics_primitives + 1));
                        }

                        out_data->skinned_meshes[m].alpha_graphics_primitives[out_data->skinned_meshes[m].num_alpha_graphics_primitives] = out_data->skinned_graphics_primitives + gp;
                        ++out_data->skinned_meshes[m].num_alpha_graphics_primitives;
                    }

                    if (strstr (current_mgp->material->name, "blend") != NULL)
                    {
                        if (out_data->skinned_meshes[m].blend_graphics_primitives == NULL)
                        {
                            out_data->skinned_meshes[m].blend_graphics_primitives = (vk_skinned_primitive**)utils_calloc (1, sizeof (vk_skinned_primitive*));
                        }
                        else
                        {
                            out_data->skinned_meshes[m].blend_graphics_primitives = (vk_skinned_primitive**)utils_realloc_zero (out_data->skinned_meshes[m].blend_graphics_primitives, sizeof (vk_skinned_primitive*) * out_data->skinned_meshes[m].num_blend_graphics_primitives, sizeof (vk_skinned_primitive*) * (out_data->skinned_meshes[m].num_blend_graphics_primitives + 1));
                        }

                        out_data->skinned_meshes[m].blend_graphics_primitives[out_data->skinned_meshes[m].num_blend_graphics_primitives] = out_data->skinned_graphics_primitives + gp;
                        ++out_data->skinned_meshes[m].num_blend_graphics_primitives;
                    }
                }
            }
        }
    }

    for (size_t m = 0; m < num_ref_cgltf_static_mesh_nodes; ++m)
    {
        cgltf_node* current_node = ref_cgltf_static_mesh_nodes[m];
        for (size_t mgp = 0; mgp < current_node->mesh->primitives_count; ++mgp)
        {
            cgltf_primitive* current_mgp = current_node->mesh->primitives + mgp;
            for (size_t gp = 0; gp < num_ref_cgltf_static_graphics_primitives; ++gp)
            {
                cgltf_primitive* current_gp = ref_cgltf_static_graphics_primitives[gp];

                if (current_mgp == current_gp)
                {
                    if (strstr (current_mgp->material->name, "opaque") != NULL)
                    {
                        if (out_data->static_meshes[m].opaque_graphics_primitives == NULL)
                        {
                            out_data->static_meshes[m].opaque_graphics_primitives = (vk_static_primitive**)utils_calloc (1, sizeof (vk_static_primitive*));
                        }
                        else
                        {
                            out_data->static_meshes[m].opaque_graphics_primitives = (vk_static_primitive**)utils_realloc_zero (out_data->static_meshes[m].opaque_graphics_primitives, sizeof (vk_static_primitive*) * out_data->static_meshes[m].num_opaque_graphics_primitives, sizeof (vk_static_primitive*) * (out_data->static_meshes[m].num_opaque_graphics_primitives + 1));
                        }

                        out_data->static_meshes[m].opaque_graphics_primitives[out_data->static_meshes[m].num_opaque_graphics_primitives] = out_data->static_graphics_primitives + gp;
                        ++out_data->static_meshes[m].num_opaque_graphics_primitives;
                    }

                    if (strstr (current_mgp->material->name, "alpha") != NULL)
                    {
                        if (out_data->static_meshes[m].alpha_graphics_primitives == NULL)
                        {
                            out_data->static_meshes[m].alpha_graphics_primitives = (vk_static_primitive**)utils_calloc (1, sizeof (vk_static_primitive*));
                        }
                        else
                        {
                            out_data->static_meshes[m].alpha_graphics_primitives = (vk_static_primitive**)utils_realloc_zero (out_data->static_meshes[m].alpha_graphics_primitives, sizeof (vk_static_primitive*) * out_data->static_meshes[m].num_alpha_graphics_primitives, sizeof (vk_static_primitive*) * (out_data->static_meshes[m].num_alpha_graphics_primitives + 1));
                        }

                        out_data->static_meshes[m].alpha_graphics_primitives[out_data->static_meshes[m].num_alpha_graphics_primitives] = out_data->static_graphics_primitives + gp;
                        ++out_data->static_meshes[m].num_alpha_graphics_primitives;
                    }

                    if (strstr (current_mgp->material->name, "blend") != NULL)
                    {
                        if (out_data->static_meshes[m].blend_graphics_primitives == NULL)
                        {
                            out_data->static_meshes[m].blend_graphics_primitives = (vk_static_primitive**)utils_calloc (1, sizeof (vk_static_primitive*));
                        }
                        else
                        {
                            out_data->static_meshes[m].blend_graphics_primitives = (vk_static_primitive**)utils_realloc_zero (out_data->static_meshes[m].blend_graphics_primitives, sizeof (vk_static_primitive*) * out_data->static_meshes[m].num_blend_graphics_primitives, sizeof (vk_static_primitive*) * (out_data->static_meshes[m].num_blend_graphics_primitives + 1));
                        }

                        out_data->static_meshes[m].blend_graphics_primitives[out_data->static_meshes[m].num_blend_graphics_primitives] = out_data->static_graphics_primitives + gp;
                        ++out_data->static_meshes[m].num_blend_graphics_primitives;
                    }
                }
            }
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_skins (cgltf_data** datas, size_t num_datas, scene_graphics* out_data)
{
    OutputDebugString (L"import_skins\n");

    size_t num_skins = 0;
    size_t current_skin_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        if (ref_cgltf_skins == NULL)
        {
            ref_cgltf_skins = (cgltf_skin**)utils_calloc (current_data->skins_count, sizeof (cgltf_skin*));
        }
        else
        {
            ref_cgltf_skins = (cgltf_skin**)utils_realloc_zero (ref_cgltf_skins, sizeof (cgltf_skin*) * out_data->num_skins, sizeof (cgltf_skin*) * (out_data->num_skins + current_data->skins_count));
        }

        if (out_data->skins == NULL)
        {
            out_data->skins = (vk_skin*)utils_calloc (current_data->skins_count, sizeof (vk_skin));
        }
        else
        {
            out_data->skins = (vk_skin*)utils_realloc_zero (out_data->skins, sizeof (vk_skin) * out_data->num_skins, sizeof (vk_skin) * (out_data->num_skins + current_data->skins_count));
        }

        for (size_t s = 0; s < current_data->skins_count; ++s)
        {
            ref_cgltf_skins[current_skin_index] = current_data->skins + s;
            ++current_skin_index;
        }

        num_ref_cgltf_skins += current_data->skins_count;
        out_data->num_skins += current_data->skins_count;
        num_skins += current_data->skins_count;
    }

    size_t skin_joints_size = MAX_JOINTS * sizeof (float) * 16;
    size_t min_ubo_alignment = (size_t)physical_device_limits.minUniformBufferOffsetAlignment;
    size_t aligned_skin_joints_size = 0;
    vk_utils_get_aligned_size (skin_joints_size, min_ubo_alignment, &aligned_skin_joints_size);
    
    size_t total_data_size = aligned_skin_joints_size * num_skins;

    float* skin_joints_matrices = (float*)utils_aligned_malloc_zero (total_data_size, min_ubo_alignment);

    for (size_t s = 0; s < num_ref_cgltf_skins; ++s)
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

    AGAINST_RESULT result = AGAINST_SUCCESS;

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

AGAINST_RESULT link_skinned_meshes_to_skins (scene_graphics* out_data)
{
    OutputDebugString (L"link_skinned_meshes_to_skins\n");

    for (size_t s = 0; s < num_ref_cgltf_skins; ++s)
    {
        cgltf_skin* current_skin = ref_cgltf_skins[s];
        for (size_t m = 0; m < num_ref_cgltf_skinned_mesh_nodes; ++m)
        {
            cgltf_node* current_node = ref_cgltf_skinned_mesh_nodes[m];
            if (current_skin == current_node->skin)
            {
                if (out_data->skins[s].skinned_meshes == NULL)
                {
                    out_data->skins[s].skinned_meshes = (vk_skinned_mesh**)utils_calloc (1, sizeof (vk_skinned_mesh*));
                }
                else
                {
                    out_data->skins[s].skinned_meshes = (vk_skinned_mesh**)utils_realloc_zero (out_data->skins[s].skinned_meshes, sizeof (vk_skinned_mesh*) * out_data->skins[s].num_skinned_meshes, sizeof (vk_skinned_mesh*) * (out_data->skins[s].num_skinned_meshes + 1));
                }

                out_data->skins[s].skinned_meshes[out_data->skins[s].num_skinned_meshes] = out_data->skinned_meshes + m;
                ++out_data->skins[s].num_skinned_meshes;
            }
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_animations (cgltf_data** datas, size_t num_datas, scene_graphics* out_data)
{
    OutputDebugString (L"import_animations\n");

    anim_joint_data* anim_joint_datas = NULL;
    size_t num_anim_joint_datas = 0;

    for (size_t d = 0; d < num_datas; ++d)
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
                anim_joint_datas = (anim_joint_data*)utils_realloc_zero (anim_joint_datas, sizeof (anim_joint_data) * num_anim_joint_datas, sizeof (anim_joint_data) * (num_anim_joint_datas + 1));
            }

            cgltf_animation* current_animation = current_data->animations + a;
            anim_joint_data* current_ajd = anim_joint_datas + num_anim_joint_datas;

            strcpy (current_ajd->name, current_animation->name);

            for (size_t c = 0; c < current_animation->channels_count; ++c)
            {
                cgltf_animation_channel* current_channel = current_animation->channels + c;
                size_t joint_data_index = -1;
                for (size_t ja = 0; ja < current_ajd->num_joint_anims; ++ja)
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
                        current_ajd->joint_anims = (joint_data*)utils_realloc_zero (current_ajd->joint_anims, sizeof (joint_data) * current_ajd->num_joint_anims, sizeof (joint_data) * (current_ajd->num_joint_anims + 1));
                    }

                    joint_data_index = current_ajd->num_joint_anims;
                    ++current_ajd->num_joint_anims;
                }

                joint_data* active_jd = current_ajd->joint_anims + joint_data_index;

                strcpy (active_jd->joint_name, current_channel->target_node->name);
                strcpy (active_jd->anim_name, current_animation->name);

                if (current_channel->target_path == cgltf_animation_path_type_translation)
                {
                    active_jd->translations = (float*)((unsigned char*)current_channel->sampler->output->buffer_view->buffer->data + current_channel->sampler->output->offset + current_channel->sampler->output->buffer_view->offset);
                    active_jd->num_translations = current_channel->sampler->output->count;
                }

                if (current_channel->target_path == cgltf_animation_path_type_rotation)
                {
                    active_jd->rotations = (float*)((unsigned char*)current_channel->sampler->output->buffer_view->buffer->data + current_channel->sampler->output->offset + current_channel->sampler->output->buffer_view->offset);
                    active_jd->num_rotations = current_channel->sampler->output->count;
                }
            }

            ++num_anim_joint_datas;
        }
    }

    for (size_t ajd = 0; ajd < num_anim_joint_datas; ++ajd)
    {
        anim_joint_data* current_ajd = anim_joint_datas + ajd;
        for (size_t jd = 0; jd < current_ajd->num_joint_anims; ++jd)
        {
            joint_data* current_jd = current_ajd->joint_anims + jd;

            current_jd->num_matrices = current_jd->num_translations >= current_jd->num_rotations ? current_jd->num_translations : current_jd->num_rotations;
            current_jd->matrices = (float*)utils_malloc_zero (sizeof (float) * 16 * current_jd->num_matrices);

            current_ajd->num_frames = current_jd->num_matrices >= current_ajd->num_frames ? current_jd->num_matrices : current_ajd->num_frames;

            float matrix[16];

            for (size_t f = 0; f < current_jd->num_matrices; ++f)
            {
                math_create_identity_matrix (matrix);
                math_translate_matrix (matrix, current_jd->translations + (f * 3), matrix);
                math_rotate_matrix (matrix, current_jd->rotations + (f * 4), matrix);

                memcpy (current_jd->matrices + (f * 16), matrix, sizeof (float) * 16);
            }
        }
    }

    out_data->num_animations = num_anim_joint_datas;
    out_data->animations = (vk_animation*)utils_calloc (out_data->num_animations, sizeof (vk_animation));

    size_t size_of_single_frame = MAX_JOINTS * 16 * sizeof (float);
    size_t aligned_size_of_single_frame = 0;
    vk_utils_get_aligned_size (size_of_single_frame, (size_t)physical_device_limits.minUniformBufferOffsetAlignment, &aligned_size_of_single_frame);

    unsigned char* all_animations_aligned_memory = NULL;
    size_t total_data_allocated = 0;

    for (size_t ajd = 0; ajd < num_anim_joint_datas; ++ajd)
    {
        anim_joint_data* current_ajd = anim_joint_datas + ajd;
        vk_animation* current_out_anim = out_data->animations + ajd;

        strcpy (current_out_anim->name, current_ajd->name);
        current_out_anim->num_frames = current_ajd->num_frames;
        current_out_anim->frame_data_offsets = (VkDeviceSize*)utils_calloc (current_out_anim->num_frames, sizeof (VkDeviceSize));

        if (all_animations_aligned_memory == NULL)
        {
            all_animations_aligned_memory = (unsigned char*)utils_aligned_malloc_zero (size_of_single_frame * current_out_anim->num_frames, (size_t)physical_device_limits.minUniformBufferOffsetAlignment);
        }
        else
        {
            all_animations_aligned_memory = (unsigned char*)utils_aligned_realloc_zero (all_animations_aligned_memory, (size_t)physical_device_limits.minUniformBufferOffsetAlignment, total_data_allocated, total_data_allocated + (size_of_single_frame * current_out_anim->num_frames));
        }
            
        for (size_t f = 0; f < current_ajd->num_frames; ++f)
        {
            for (size_t jd = 0; jd < current_ajd->num_joint_anims; ++jd)
            {
                joint_data* current_jd = current_ajd->joint_anims + jd;
                float* joint_matrix = current_jd->matrices + (f * 16);

                memcpy (all_animations_aligned_memory + (ajd * size_of_single_frame) + (jd * sizeof (float) * 16), joint_matrix, sizeof (float) * 16);
            }

            current_out_anim->frame_data_offsets[f] = total_data_allocated;
        }

        total_data_allocated += size_of_single_frame * current_out_anim->num_frames;
    }

    AGAINST_RESULT result = AGAINST_SUCCESS;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (graphics_device, total_data_allocated, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &staging_buffer), result);
    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (graphics_device, &staging_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_buffer_memory), result);
    CHECK_AGAINST_RESULT (vk_utils_map_data_to_device_memory (graphics_device, staging_buffer_memory, 0, total_data_allocated, all_animations_aligned_memory), result);
    CHECK_AGAINST_RESULT (vk_utils_create_buffer (graphics_device, total_data_allocated, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &out_data->anim_buffer), result);
    CHECK_AGAINST_RESULT (vk_utils_allocate_bind_buffer_memory (graphics_device, &out_data->anim_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &out_data->anim_buffer_memory), result);
    CHECK_AGAINST_RESULT (vk_utils_copy_buffer_to_buffer (graphics_device, transfer_command_pool, transfer_queue, staging_buffer, out_data->anim_buffer, total_data_allocated), result);

    vk_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_buffer_memory);

    for (size_t ajd = 0; ajd < num_anim_joint_datas; ++ajd)
    {
        for (size_t jd = 0; jd < anim_joint_datas[ajd].num_joint_anims; ++jd)
        {
            utils_free (anim_joint_datas[ajd].joint_anims[jd].matrices);
        }
        utils_free (anim_joint_datas[ajd].joint_anims);
    }
    utils_free (anim_joint_datas);

    utils_aligned_free (all_animations_aligned_memory);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT link_animations_to_skins (scene_graphics* out_data)
{
    OutputDebugString (L"link_animations_to_skins\n");

    for (size_t s = 0; s < out_data->num_skins; ++s)
    {
        vk_skin* current_skin = out_data->skins + s;
        for (size_t a = 0; a < out_data->num_animations; ++a)
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
                    current_skin->animations = (vk_animation**)utils_realloc_zero (current_skin->animations, sizeof (vk_animation*) * out_data->num_animations, sizeof (vk_animation*) * (out_data->num_animations + 1));
                }

                current_skin->animations[current_skin->num_animations] = out_data->animations + a;
                ++current_skin->num_animations;
            }
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_static_meshes (cgltf_data** datas, size_t num_datas, scene_graphics* out_data)
{
    OutputDebugString (L"import_static_meshes\n");

    size_t current_mesh_node_index = 0;

    for (size_t d = 0; d < num_datas; ++d)
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

            if (current_node->skin != NULL)
            {
                continue;
            }

            if (out_data->static_meshes == NULL)
            {
                out_data->static_meshes = (vk_static_mesh*)utils_calloc (1, sizeof (vk_static_mesh));
            }
            else
            {
                out_data->static_meshes = (vk_static_mesh*)utils_realloc_zero (out_data->static_meshes, sizeof (vk_static_mesh) * out_data->num_static_meshes, sizeof (vk_static_mesh) * (out_data->num_static_meshes + 1));
            }

            if (ref_cgltf_static_mesh_nodes == NULL)
            {
                ref_cgltf_static_mesh_nodes = (cgltf_node**)utils_calloc (1, sizeof (cgltf_node*));
            }
            else
            {
                ref_cgltf_static_mesh_nodes = (cgltf_node**)utils_realloc_zero (ref_cgltf_static_mesh_nodes, sizeof (cgltf_node*) * out_data->num_static_meshes, sizeof (cgltf_node*) * (out_data->num_static_meshes + 1));
            }

            strcpy (out_data->static_meshes[current_mesh_node_index].name, current_node->name);

            ref_cgltf_static_mesh_nodes[current_mesh_node_index] = current_node;
            ++num_ref_cgltf_static_mesh_nodes;
            ++out_data->num_static_meshes;
            ++current_mesh_node_index;
        }
    }

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_physics_primitives (cgltf_data** datas, size_t num_datas, scene_physics* out_data)
{
    OutputDebugString (L"import_physics_primitives\n");

    return AGAINST_SUCCESS;
}

AGAINST_RESULT link_physics_primitives_to_meshes (scene_physics* out_data)
{
    OutputDebugString (L"link_physics_primitives_to_meshes\n");

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_static_physics_primitives (cgltf_data** datas, size_t num_datas, scene_physics* out_data)
{
    OutputDebugString (L"import_static_graphics_primitives\n");

    return AGAINST_SUCCESS;
}

AGAINST_RESULT link_static_physics_primitives_to_static_meshes (scene_physics* out_data)
{
    OutputDebugString (L"link_static_physics_primitives_to_static_meshes\n");

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_graphics_from_gltf_datas (const char* full_folder_path, cgltf_data** datas, size_t num_datas, scene_graphics* out_data)
{
    OutputDebugString (L"import_graphics_from_gltf_datas\n");

    AGAINST_RESULT result = AGAINST_SUCCESS;
    
    CHECK_AGAINST_RESULT (import_images (full_folder_path, datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_materials (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_graphics_primitives (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (link_materials_to_graphics_primitives(out_data), result);
    CHECK_AGAINST_RESULT (import_meshes (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (link_graphics_primitives_to_meshes (out_data), result);
    CHECK_AGAINST_RESULT (import_skins (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (link_skinned_meshes_to_skins (out_data), result);
    CHECK_AGAINST_RESULT (import_animations (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (link_animations_to_skins (out_data), result);

    return AGAINST_SUCCESS;
}

AGAINST_RESULT import_physics_from_gltf_datas (const char* full_folder_path, cgltf_data** datas, size_t num_datas, scene_physics* out_data)
{
    OutputDebugString (L"import_physics_from_gltf_datas\n");

    AGAINST_RESULT result = AGAINST_SUCCESS;

    CHECK_AGAINST_RESULT (import_physics_primitives (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (link_physics_primitives_to_meshes (out_data), result);

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

AGAINST_RESULT gltf_import_scene_data_from_files_from_folder (const char* partial_folder_path, scene* out_data)
{
    OutputDebugString (L"gltf_import_scene_data_from_files_from_folder\n");
    
    file_path* file_paths = NULL;
    size_t num_files = 0;
    utils_get_files_in_folder (partial_folder_path, &file_paths, &num_files);

    num_gltf_datas = num_files;
    gltf_datas = (cgltf_data**)utils_calloc (num_files, sizeof (cgltf_data*));

    char full_folder_path[MAX_PATH];
    utils_get_full_file_path (partial_folder_path, full_folder_path);

    AGAINST_RESULT result = AGAINST_SUCCESS;

    for (size_t f = 0 ; f < num_files; ++f)
    {
        char current_full_file_path[MAX_PATH];
        strcpy (current_full_file_path, full_folder_path);
        strcat (current_full_file_path, file_paths[f].path);
        CHECK_AGAINST_RESULT (gather_gltf_datas (current_full_file_path, gltf_datas, f), result);
    }

    utils_free (file_paths);
    file_paths = NULL;

    CHECK_AGAINST_RESULT (import_graphics_from_gltf_datas (full_folder_path, gltf_datas, num_gltf_datas, out_data->graphics), result);
    CHECK_AGAINST_RESULT (import_physics_from_gltf_datas (full_folder_path, gltf_datas, num_gltf_datas, out_data->physics), result);

    for (size_t d = 0; d < num_gltf_datas; ++d)
    {
        cgltf_free (gltf_datas[d]);
    }

    utils_free (gltf_datas);
    gltf_datas = NULL;
    num_gltf_datas = 0;

    utils_free (ref_cgltf_images);
    ref_cgltf_images = NULL;
    num_ref_cgltf_images = 0;

    utils_free (ref_cgltf_materials);
    ref_cgltf_materials = NULL;
    num_ref_cgltf_materials = 0;

    utils_free (ref_cgltf_anims);
    ref_cgltf_anims = NULL;
    num_ref_cgltf_anims = 0;

    utils_free (ref_cgltf_skinned_graphics_primitives);
    ref_cgltf_skinned_graphics_primitives = NULL;
    num_ref_cgltf_skinned_graphics_primitives = 0;

    utils_free (ref_cgltf_static_graphics_primitives);
    ref_cgltf_static_graphics_primitives = NULL;
    num_ref_cgltf_static_graphics_primitives = 0;

    utils_free (ref_cgltf_joints);
    ref_cgltf_joints = NULL;
    num_ref_cgltf_joints = 0;

    utils_free (ref_cgltf_skins);
    ref_cgltf_skins = NULL;
    num_ref_cgltf_skins = 0;

    utils_free (ref_cgltf_skinned_mesh_nodes);
    ref_cgltf_skinned_mesh_nodes = NULL;
    num_ref_cgltf_skinned_mesh_nodes = 0;

    utils_free (ref_cgltf_static_mesh_nodes);
    ref_cgltf_static_mesh_nodes = NULL;
    num_ref_cgltf_static_mesh_nodes = 0;

    return AGAINST_SUCCESS;
}

AGAINST_RESULT glt_import_physics_from_files_from_folder (const char* partial_folder_path, scene_physics* out_data)
{
    return AGAINST_SUCCESS;
}