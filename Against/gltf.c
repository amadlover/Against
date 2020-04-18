#include "gltf.h"
#include "utils.h"
#include "error.h"
#include "common_graphics.h"
#include "graphics_utils.h"

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

#include <stb_image.h>

cgltf_data** gltf_datas = NULL;
size_t num_gltf_datas = 0;

cgltf_image** ref_cgltf_images_for_materials = NULL;
size_t num_ref_cgltf_images_for_materials = 0;

cgltf_material** ref_cgltf_materials_for_graphics_primitives = NULL;
size_t num_ref_cgltf_materials_for_graphics_primitives = 0;

cgltf_animation** ref_cgltf_anims_for_meshes = NULL;
size_t num_ref_cgltf_anims_for_meshes = 0;


int import_images (const char* full_folder_path, cgltf_data** datas, size_t num_datas, gltf_asset_data* out_data)
{
    size_t total_image_size = 0;

    size_t* img_offsets = NULL;
    size_t* img_sizes = NULL;
    size_t* img_widths = NULL;
    size_t* img_heights = NULL;
    uint8_t** img_pixels = NULL;

    AGAINSTRESULT result;
    for (size_t d = 0; d < num_datas; ++d)
    {
        if (out_data->images == NULL)
        {
            out_data->images = (VkImage*)my_calloc (datas[d]->images_count, sizeof (VkImage));
        }
        else
        {
            out_data->images = (VkImage*)my_realloc (out_data->images, sizeof (VkImage) * (num_ref_cgltf_images_for_materials + datas[d]->images_count));
        }

        if (out_data->image_views == NULL)
        {
            out_data->image_views = (VkImageView*)my_calloc (datas[d]->images_count, sizeof (VkImageView));
        }
        else
        {
            out_data->image_views = (VkImageView*)my_realloc (out_data->image_views, sizeof (VkImageView) * (num_ref_cgltf_images_for_materials + datas[d]->images_count));
        }

        if (img_offsets == NULL)
        {
            img_offsets = (size_t*)my_calloc (datas[d]->images_count, sizeof (size_t));
        }
        else
        {
            img_offsets = (size_t*)my_realloc (img_offsets, sizeof (size_t) * (num_ref_cgltf_images_for_materials + datas[d]->images_count));
        }

        if (img_sizes == NULL)
        {
            img_sizes = (size_t*)my_calloc (datas[d]->images_count, sizeof (size_t));
        }
        else
        {
            img_sizes = (size_t*)my_realloc (img_sizes, sizeof (size_t) * (num_ref_cgltf_images_for_materials + datas[d]->images_count));
        }

        if (img_widths == NULL)
        {
            img_widths = (size_t*)my_calloc (datas[d]->images_count, sizeof (size_t));
        }
        else
        {
            img_widths = (size_t*)my_realloc (img_widths, sizeof (size_t) * (num_ref_cgltf_images_for_materials + datas[d]->images_count));
        }

        if (img_heights == NULL)
        {
            img_heights = (size_t*)my_calloc (datas[d]->images_count, sizeof (size_t));
        }
        else
        {
            img_heights = (size_t*)my_realloc (img_heights, sizeof (size_t) * (num_ref_cgltf_images_for_materials + datas[d]->images_count));
        }

        if (img_pixels == NULL)
        {
            img_pixels = (uint8_t**)my_calloc (datas[d]->images_count, sizeof (uint8_t*));
        }
        else
        {
            img_pixels = (uint8_t**)my_realloc (img_pixels, sizeof (uint8_t*) * (num_ref_cgltf_images_for_materials + datas[d]->images_count));
        }

        if (ref_cgltf_images_for_materials == NULL)
        {
            ref_cgltf_images_for_materials = (cgltf_image**)my_calloc (datas[d]->images_count, sizeof (cgltf_image*));
        }
        else
        {
            ref_cgltf_images_for_materials = (cgltf_image**)my_realloc (ref_cgltf_images_for_materials, sizeof (cgltf_image*) * (datas[d]->images_count + num_ref_cgltf_images_for_materials));
        }

        for (size_t i = 0; i < datas[d]->images_count; ++i)
        {
            size_t current_index = num_ref_cgltf_images_for_materials + i;

            cgltf_image* image = datas[d]->images + i;
            char full_texture_path[MAX_PATH];
            get_full_texture_path_from_uri (full_folder_path, image->uri, full_texture_path);
            int bpp;
            img_pixels[current_index] = stbi_load (full_texture_path, img_widths + current_index, img_heights + current_index, &bpp, 4);

            VkExtent3D image_extent = { img_widths[current_index], img_heights[current_index], 1 };
            CHECK_AGAINST_RESULT (graphics_utils_create_image (graphics_device, graphics_queue_family_index, image_extent, 1, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_SHARING_MODE_EXCLUSIVE, &out_data->images[current_index]), result);
            img_sizes[current_index] = img_widths[current_index] * img_heights[current_index] * 4;
            total_image_size += img_sizes[current_index];
            img_offsets[current_index] = current_index > 0 ? total_image_size - img_sizes[current_index] : 0;

            ref_cgltf_images_for_materials[current_index] = image;
        }

        num_ref_cgltf_images_for_materials += datas[d]->images_count;
        out_data->images_count += datas[d]->images_count;
    }

    VkBuffer staging_buffer; VkDeviceMemory staging_memory;
    CHECK_AGAINST_RESULT (graphics_utils_create_buffer (graphics_device, total_image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, graphics_queue_family_index, &staging_buffer), result);
    CHECK_AGAINST_RESULT (graphics_utils_allocate_bind_buffer_memory (graphics_device, &staging_buffer, 1, physical_device_memory_properties, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &staging_memory), result);

    for (size_t i = 0; i < num_ref_cgltf_images_for_materials; ++i)
    {
        CHECK_AGAINST_RESULT (graphics_utils_map_data_to_device_memory (graphics_device, staging_memory, img_offsets[i], img_sizes[i], img_pixels[i]), result);
        CHECK_AGAINST_RESULT (graphics_utils_change_image_layout (graphics_device, graphics_queue, command_pool, graphics_queue_family_index, out_data->images[i], 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT), result);
    }

    CHECK_AGAINST_RESULT (graphics_utils_allocate_bind_image_memory (graphics_device, out_data->images, num_ref_cgltf_images_for_materials, physical_device_memory_properties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &out_data->images_memory), result);

    for (size_t i = 0; i < num_ref_cgltf_images_for_materials; ++i)
    {
        VkExtent3D img_extent = { img_widths[i], img_heights[i], 1 };
        CHECK_AGAINST_RESULT (graphics_utils_copy_buffer_to_image (graphics_device, command_pool, graphics_queue, img_offsets[i], staging_buffer, &out_data->images[i], img_extent, 1), result);
        CHECK_AGAINST_RESULT (graphics_utils_change_image_layout (graphics_device, graphics_queue, command_pool, graphics_queue_family_index, out_data->images[i], 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT), result);
        CHECK_AGAINST_RESULT (graphics_utils_create_image_view (graphics_device, out_data->images[i], &out_data->image_views[i]), result);
    }

    my_free (img_offsets);
    my_free (img_sizes);
    my_free (img_widths);
    my_free (img_heights);
    
    for (size_t i = 0; i < num_ref_cgltf_images_for_materials; ++i)
    {
        stbi_image_free (img_pixels[i]);
    }

    graphics_utils_destroy_buffer_and_buffer_memory (graphics_device, staging_buffer, staging_memory);

    return 0;
}

int import_materials (cgltf_data** datas, size_t num_datas, gltf_asset_data* out_data)
{
    for (size_t d = 0; d < num_datas; ++d)
    {
        if (out_data->materials == NULL)
        {
            out_data->materials = (vk_material*)my_calloc (datas[d]->materials_count, sizeof (vk_material));
        }
        else
        {
            out_data->materials = (vk_material*)my_realloc (out_data->materials, sizeof (vk_material) * (datas[d]->materials_count + out_data->materials_count));
        }

        if (ref_cgltf_materials_for_graphics_primitives == NULL)
        {
            ref_cgltf_materials_for_graphics_primitives = (cgltf_material**)my_calloc (datas[d]->materials_count, sizeof (cgltf_material*));
        }
        else
        {
            ref_cgltf_materials_for_graphics_primitives = (cgltf_material**)my_realloc (ref_cgltf_materials_for_graphics_primitives, sizeof (cgltf_material*) * (out_data->materials_count + datas[d]->materials_count));
        }

        for (size_t m = 0; m < datas[d]->materials_count; ++m)
        {
            size_t current_index = out_data->materials_count + m;
            cgltf_material* material = datas[d]->materials + m;

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

            ref_cgltf_materials_for_graphics_primitives[current_index] = material;
        }

        out_data->materials_count += datas[d]->materials_count;
    }

    my_free (ref_cgltf_images_for_materials);

    return 0;
}

int import_animations (cgltf_data** datas, size_t num_datas, gltf_asset_data* out_data)
{
    return 0;
}

int import_skins (cgltf_data** datas, size_t num_datas, gltf_asset_data* out_data)
{
    for (size_t d = 0; d < num_datas; ++d)
    {
        if (out_data->skins == NULL)
        {
            out_data->skins = (vk_skin*)my_calloc (datas[d]->skins_count, sizeof (vk_skin));
        }
        else
        {
            out_data->skins = (vk_skin*)my_realloc (out_data->skins, sizeof (vk_skin) * (datas[d]->skins_count + out_data->skins_count));
        }
    }

    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        for (size_t s = 0; s < current_data->skins_count; ++s)
        {
            cgltf_skin* current_skin = current_data->skins + s;

            for (size_t j = 0; j < current_skin->joints_count; ++j)
            {
                cgltf_node* current_joint = current_skin->joints[j];
                
                wchar_t ski_joints[2048];
                swprintf (ski_joints, 2048, L"Skin: %hs -> Joint: %hs -> Parent: %hs\n", current_skin->name, current_joint->name, current_joint->parent == NULL ? "" : current_joint->parent->name);
             //   OutputDebugString (ski_joints);
            }
        }
    }

    return 0;
}

int import_graphics_meshes (cgltf_data** datas, size_t num_datas, gltf_asset_data* out_data)
{
    for (size_t d = 0; d < num_datas; ++d)
    {
        cgltf_data* current_data = datas[d];

        for (size_t n = 0; n < current_data->nodes_count; ++n)
        {
            cgltf_node* current_node = current_data->nodes + n;

            if (current_node->mesh == NULL) continue;
            if (strstr (current_node->name, "CS_") != NULL) continue;


            if (out_data->skeletal_meshes == NULL)
            {
                out_data->skeletal_meshes = (vk_skeletal_mesh*)my_calloc (1, sizeof (vk_skeletal_mesh));
            }
            else
            {
                out_data->skeletal_meshes = (vk_skeletal_mesh*)my_realloc (out_data->skeletal_meshes, sizeof (vk_skeletal_mesh) * (out_data->skeletal_mesh_count + 1));
            }
            
            size_t current_mesh_index = out_data->skeletal_mesh_count;
            
            cgltf_mesh* current_mesh = current_node->mesh;
            strcpy (out_data->skeletal_meshes[current_mesh_index].name, current_mesh->name);
            out_data->skeletal_meshes[current_mesh_index].num_skeletal_graphics_primitives = current_mesh->primitives_count;
            out_data->skeletal_meshes[current_mesh_index].skeletal_graphics_primitives = (vk_skeletal_graphics_primitive*)my_calloc (current_mesh->primitives_count, sizeof (vk_skeletal_graphics_primitive));

            for (size_t p = 0; p < current_mesh->primitives_count; ++p)
            {
                cgltf_primitive* current_primitive = current_mesh->primitives + p;
            }

            ++out_data->skeletal_mesh_count;
        }
    }

    my_free (ref_cgltf_materials_for_graphics_primitives);

    return 0;
}

int import_gltf_datas (const char* full_folder_path, cgltf_data** datas, size_t num_datas, gltf_asset_data* out_data)
{
    AGAINSTRESULT result;
    
    CHECK_AGAINST_RESULT (import_images (full_folder_path, datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_materials (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_animations (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_skins (datas, num_datas, out_data), result);
    CHECK_AGAINST_RESULT (import_graphics_meshes (datas, num_datas, out_data), result);

    return 0;
}

int gather_gltf_datas (const char* full_file_path, cgltf_data** datas, size_t current_data_index)
{
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

int import_gltf_files_from_folder (const char* partial_folder_path, gltf_asset_data** out_gltf_data)
{
    file_path* file_paths;
    size_t num_files = 0;
    get_files_in_folder (partial_folder_path, &file_paths, &num_files);

    num_gltf_datas = num_files;
    gltf_datas = (cgltf_data**)my_calloc (num_files, sizeof (cgltf_data*));

    char full_folder_path[MAX_PATH];
    get_full_file_path (partial_folder_path, full_folder_path);

    AGAINSTRESULT result;

    for (size_t f = 0 ; f < num_files; ++f)
    {
        char current_full_file_path[MAX_PATH];
        strcpy (current_full_file_path, full_folder_path);
        strcat (current_full_file_path, file_paths[f].path);
        CHECK_AGAINST_RESULT (gather_gltf_datas (current_full_file_path, gltf_datas, f), result);
    }

    my_free (file_paths);

    CHECK_AGAINST_RESULT (import_gltf_datas (full_folder_path, gltf_datas, num_gltf_datas, *out_gltf_data), result);

    for (size_t d = 0; d < num_gltf_datas; ++d)
    {
        cgltf_free (gltf_datas[d]);
    }

    my_free (gltf_datas);

    return 0;
}

void cleanup_gltf_data (gltf_asset_data* gltf_data)
{
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
            my_free (gltf_data->images);
        }

        graphics_utils_destroy_buffer_and_buffer_memory (graphics_device, gltf_data->vb_ib, gltf_data->vb_ib_memory);
        vkFreeMemory (graphics_device, gltf_data->images_memory, NULL);

        if (gltf_data->skeletal_meshes)
        {
            for (size_t m = 0; m < gltf_data->skeletal_mesh_count; ++m)
            {
                vk_skeletal_mesh* current_mesh = gltf_data->skeletal_meshes + m;
                my_free (current_mesh->skeletal_graphics_primitives);
            }

            my_free (gltf_data->skeletal_meshes);
        }

        my_free (gltf_data->skins);
        my_free (gltf_data->materials);
        my_free (gltf_data->animations);

        my_free (gltf_data);
    }
}